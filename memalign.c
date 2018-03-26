#include <stdio.h>
#include "dsnf.h"

void *memalign(size_t alignment, size_t size) {

  void *ptr = NULL;

  if(alignment && size) {
    size_t offset = alignment - 1;
    void *memory1 = malloc(size + offset);
    void *memory2 = malloc(size);
    if(memory1 && memory2) {
      if((uintptr_t)memory2 % alignment != 0) {
        ptr = (void*)((uintptr_t)memory1 + alignment - ((uintptr_t)memory1 % alignment));
        memcpy(memory2, ptr, size);
        free_memory(memory1);
        return memory2;
      }
      return memory2;
    }
  }
  return ptr;
}
