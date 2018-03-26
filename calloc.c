#include <stdio.h>
#include "dsnf.h"

void *calloc(size_t nmemb, size_t size) {
  if(nmemb==0||size==0) {
    return NULL;
  }
  void *ptr;
  size_t finalsize = nmemb * size;
  if((ptr = get_memory(finalsize)) == NULL) {
    return NULL;
  }
  memset(ptr, 0, finalsize);
  return ptr;
}
