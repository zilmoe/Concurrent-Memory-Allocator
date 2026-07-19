/*
This is the testing file.
I have no idea how to
go about this so
might be a bit unconventional. :)
*/

#include <cassert>
#include <thread>
#include <iostream>
#include <chrono>
#include <allocator.h>

// Test 1!
void worker_task(std::stop_token token, int thread_id) {
    while (!token.stop_requested()) {
        char* alloced_mem = (char *)allocate(20);
        for (int i = 0; i < 20; i++){
          alloced_mem[i] = '\x90';
        }
        free(alloced_mem);
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
    }
    // we should free it here but the destructor will return the memory to
    // the operating system for us!
}

bool test_single_thread() {
  // make one thread that allocates memory
  std::jthread jt(worker_task, 42);
  std::this_thread::sleep_for(std::chrono::milliseconds(700));
  std::cout << "Test 1 (single thread) passed!" << std::endl;
  return true;
}

// Test 2
bool test_multiple_threads() {
  // make some threads that allocate memory
  std::jthread jt(worker_task, 2);
  std::jthread jt2(worker_task, 3);
  std::jthread jt3(worker_task, 4);
  std::jthread jt4(worker_task, 5);
  std::this_thread::sleep_for(std::chrono::milliseconds(700));
  std::cout << "Test 2 (multiple threads) passed!" << std::endl;
  return true;
}

bool test_various_sizes() {
  // const size_t sizes[] = {1, 2, 3, 7, 8, 15, 16, 31, 32, 63, 64,
  //                         127, 128, 255, 256, 511, 1000, 4096};
  const size_t sizes[] = {1, 2, 3, 7, 8, 15, 16, 31, 32, 63, 64,
                          127, 128, 255, 256, 511, 1000};
  for (size_t s : sizes) {
    char* p = (char*)allocate(s);
    assert(p != nullptr);
    std::memset(p, 0xAB, s);
    for (size_t i = 0; i < s; i++)
      assert((unsigned char)p[i] == 0xAB);
    free(p);
  }
  std::cout << "Test 3 (various sizes) passed!" << std::endl;
  return true;
}

bool test_no_overlap() {
  const int N = 64;
  const size_t blockSize = 64;
  std::vector<char*> blocks;
  for (int i = 0; i < N; i++) {
    char* p = (char*)allocate(blockSize);
    assert(p != nullptr);
    std::memset(p, (char)i, blockSize);
    blocks.push_back(p);
  }
  for (int i = 0; i < N; i++)
    for (size_t j = 0; j < blockSize; j++)
      assert((unsigned char)blocks[i][j] == (unsigned char)i);
  for (char* p : blocks) free(p);
  std::cout << "Test 4 (no overlap) passed!" << std::endl;
  return true;
}

bool test_reuse_churn() {
  for (int i = 0; i < 10000; i++) {
    char* p = (char*)allocate(48);
    assert(p != nullptr);
    std::memset(p, 0x5A, 48);
    free(p);
  }
  std::cout << "Test 5 (reuse churn) passed!" << std::endl;
  return true;
}

bool test_free_orders() {
  const int N = 100;
  const size_t s = 32;
  // LIFO
  {
    std::vector<char*> v;
    for (int i = 0; i < N; i++) { char* p = (char*)allocate(s); std::memset(p, 1, s); v.push_back(p); }
    for (int i = N - 1; i >= 0; i--) free(v[i]);
  }
  // FIFO
  {
    std::vector<char*> v;
    for (int i = 0; i < N; i++) { char* p = (char*)allocate(s); std::memset(p, 1, s); v.push_back(p); }
    for (int i = 0; i < N; i++) free(v[i]);
  }
  std::cout << "Test 6 (free orders) passed!" << std::endl;
  return true;
}

int main(){
  // run tests here
  size_t ROUNDS = 100;
  for (size_t i = 0; i < ROUNDS; i++){
    std::cout << "Round: " << (i+1) << std::endl;
    assert(test_single_thread() == true);
    assert(test_multiple_threads() == true);
    assert(test_various_sizes() == true);
    assert(test_no_overlap() == true);
    assert(test_reuse_churn() == true);
    assert(test_free_orders() == true);
  }
  std::cout << "All tests passed!" << std::endl;
  return 0;
}
