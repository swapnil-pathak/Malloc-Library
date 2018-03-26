#include <stdio.h>
#include"dsnf.h"

void *malloc(size_t size) {
  if(size <= 0) {
    return NULL;
  }
  void *ptr;
  ptr = get_memory(size);
  return ptr;
}
