/*
This is a memory allocator implemented as a memory arena.
*/

#include <sys/mman.h>
#include <unistd.h>

#include <cstdlib>
#include <mutex>
#include <sanitizer/asan_interface.h>

#include "bloom.h"

struct free_block {
  free_block* forward;
  free_block* prev;
  // the rest of the chunk is here...
};

class FreeListStack {
  void* free_list = {nullptr};
  size_t len = 0;

 public:
  FreeListStack() {}

  ~FreeListStack() {
    // idk what to do here....
  }

 public:
  void push(void* block) {
    if (!block) {
      return;
    }

    if (is_empty()) {
      free_list = block;
    }

    free_block* top_block = (free_block*)(free_list);
    top_block->prev = (free_block*)block;
    len += 1;

    ((free_block*)block)->forward = top_block;
  }

 public:
  void* pop() {
    if (is_empty()) {
      return nullptr;
    }

    free_block* new_top = ((free_block*)free_list)->forward;
    if (new_top) {
      new_top->prev = nullptr;
    }

    void* returned_block = free_list;
    free_list = (void*)new_top;
    len -= 1;

    return returned_block;
  }

 public:
  bool is_empty() { return len == 0; }
};

class BankHeap {
  std::mutex malloc_mutex;
  FreeListStack free_list = FreeListStack();
  char* memory_arena = nullptr;
  char* arena_base = nullptr;
  static const int ARENA_CAPACITY = 4096;

  // initializes memory by asking it from
  // the kernel
 public:
  BankHeap() {
    void* mem_start = mmap(nullptr, ARENA_CAPACITY, PROT_READ | PROT_WRITE,
                           MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);

    if (mem_start == MAP_FAILED) {
      // mem allocation failed.
      // crash for now (should probably not do this :D)
      std::abort();
    }

    arena_base = (char*)mem_start;
    memory_arena = (char*)mem_start;
  }

  // returns a pointer to the malloced memory
  // or nullptr if it is at max capacity
 public:
  void* alloc() {
    size_t size = 1024;  // each thread can get 1024. have to do virtual memory shenanigans
                         // later for threads to request more :)
    // check if we can grab from the free_list
    if (!free_list.is_empty()) {
      return free_list.pop();
    }

    // ( ͡• _•)
    if (((memory_arena + size) - arena_base) > ARENA_CAPACITY) {
      return nullptr;
    }

    std::lock_guard<std::mutex> lock(malloc_mutex);

    void* allocated_block = memory_arena;
    memory_arena += size;
    return allocated_block;
  }

  // thread returning a block! yipee!
 public:
   void free(void* block){
    std::lock_guard<std::mutex> lock(malloc_mutex);
    free_list.push(block);
   }

  // reset the heap
  // might be important sometimes...
  // although it does raise the question
  // of what happens to all the threads..
  // might have to not support this
  // for now
 public:
  void nuke_all() {
    // REQUIRES; thread-count = 0
    std::lock_guard<std::mutex> lock(malloc_mutex);
    memory_arena = arena_base;
  }

  // return memory to the kernel
  // same as above...
  ~BankHeap() {
    // REQUIRES; thread-count = 0
    if (arena_base != nullptr) {
      munmap(arena_base, ARENA_CAPACITY);

      arena_base = nullptr;
      memory_arena = nullptr;
    }
  }
};

BankHeap global_bank;

class ThreadCache {
  void* free_lists[64] = {nullptr};
  size_t memory_footprint = 0;  // do we need this?
  char* thread_arena = nullptr; // the top of the arena
  char* arena_base = nullptr; // the base of the arena
  static const int ARENA_CAPACITY = 1024;

 public:
  ThreadCache() {
    // request memory from the bank
    void* mem_start = global_bank.alloc();

    if (!mem_start) {
      // thread didn't get the memory it needed...
      std::abort();  // probably not the best rn... :D
    }

    ASAN_POISON_MEMORY_REGION(mem_start, ARENA_CAPACITY); // when the thread grabs memory, poison it all
    arena_base = (char*)mem_start;
    thread_arena = (char*)mem_start;
  }

  public:
    void* alloc(int size){
      if (thread_arena + size > arena_base + ARENA_CAPACITY){
        // we don't have enough memory... ABORT!
        // (same comment as other aborts :D)
        std::abort();
      }

      void* chunk = thread_arena;
      thread_arena += size;
      ASAN_UNPOISON_MEMORY_REGION(chunk, size); // we give out memory, unpoison it
      return chunk;
    }

    // this releases all the memory that is owned
    // by the user
    void free_mem(){
      thread_arena = arena_base;
      ASAN_POISON_MEMORY_REGION(arena_base, ARENA_CAPACITY);
    }

  ~ThreadCache() {
    // give memory back to the central bank
    ASAN_UNPOISON_MEMORY_REGION(arena_base, ARENA_CAPACITY);
    global_bank.free(arena_base);
  }
};

thread_local ThreadCache t_cache;

// callable functions
void *allocate(size_t size){
  return t_cache.alloc(size);
}

void free(void *chunk){
  t_cache.free_mem();
}
