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
  // make some threads that allocate memory
  std::jthread jt(worker_task, 42);
  std::this_thread::sleep_for(std::chrono::milliseconds(700));
  std::cout << "Test 1 passed!" << std::endl;
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
  std::cout << "Test 2 passed!" << std::endl;
  return true;
}

int main(){
  // run tests here
  assert(test_single_thread() == true);
  assert(test_multiple_threads() == true);
  std::cout << "All tests passed!" << std::endl;
  return 0;
}
