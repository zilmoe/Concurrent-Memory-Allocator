#include <cstddef>

#if defined(__has_feature)
#  if __has_feature(address_sanitizer)
#    define ASAN_ENABLED
#  endif
#elif defined(__SANITIZE_ADDRESS__)
#  define ASAN_ENABLED
#endif

#ifdef ASAN_ENABLED
#  include <sanitizer/asan_interface.h>
#  define ASAN_POISON_MEMORY_REGION(addr, size) __asan_poison_memory_region((addr), (size))
#  define ASAN_UNPOISON_MEMORY_REGION(addr, size) __asan_unpoison_memory_region((addr), (size))
#else
#  define ASAN_POISON_MEMORY_REGION(addr, size)
#  define ASAN_UNPOISON_MEMORY_REGION(addr, size)
#endif

// request memory
void *allocate(size_t);

// free memory
void free(void *);

