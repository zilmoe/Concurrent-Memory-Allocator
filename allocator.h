// request memory
void *bloom_malloc(int);

// free memory
void bloom_free(void *);

// return memory to kernel
void mem_destroy();

// reset the heap
void nuke_all();
