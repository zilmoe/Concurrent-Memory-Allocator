/*
This is a memory allocator implemented as a memory arena.
*/

#include <unistd.h>
#include <cstdlib>
#include <sys/mman.h>
#include <mutex>
#include "bloom.h"

class BankHeap {
  std::mutex malloc_mutex;
  char* memory_arena = nullptr;
  char* arena_base = nullptr;
  static const int ARENA_CAPACITY = 4096;

  // initializes memory by asking it from
  // the kernel
  public: BankHeap(){
    void* mem_start = mmap(
        nullptr,
        ARENA_CAPACITY,
        PROT_READ | PROT_WRITE,
        MAP_PRIVATE | MAP_ANONYMOUS,
        -1,
        0
    );

    if (mem_start == MAP_FAILED) {
        // mem allocation failed.
        // crash for now (should probably not do this :D)
        std::abort();
    }

    arena_base = (char *)mem_start;
    memory_arena = (char *)mem_start;
  }

  // return memory to the kernel
  ~BankHeap(){    
    if (arena_base != nullptr) {
        munmap(arena_base, ARENA_CAPACITY);
        
        arena_base = nullptr;
        memory_arena = nullptr;
    }
  }

  
  // returns a pointer to the malloced memory
  // or nullptr if it is at max capacity
  public: void *bloom_malloc(int size){
    if (ARENA_CAPACITY % 8 != 0){
      return nullptr;
    }

    if (((memory_arena + size) - arena_base) > ARENA_CAPACITY){
      return nullptr;
    }

    std::lock_guard<std::mutex> lock(malloc_mutex);

    void *allocated_block = memory_arena;
    memory_arena += size;
    return allocated_block;
  }

  // reset the heap
  public: void nuke_all(){
    std::lock_guard<std::mutex> lock(malloc_mutex);
    memory_arena = arena_base;
  }
};

BankHeap global_bank;

struct ThreadCache {
    void* free_lists[64] = {nullptr};    
    size_t memory_footprint = 0;

    ThreadCache() {
      // request memory from the bank
      global_bank.bloom_malloc(1024);
    }

    ~ThreadCache() {
      // give memory back to the central bank
    }
};

thread_local ThreadCache t_cache;
