#include <stdio.h>
#include "dsnf.h"

void *realloc(void *ptr, size_t size) {
  if (ptr == NULL) {
    return get_memory(size);
  }
  if (size == 0) {
    free_memory(ptr);
    return NULL;
  }
  size += sizeof(mall_h_t);
  mall_h_t *tmp = (mall_h_t *) ((char *) ptr - sizeof(mall_h_t));
  int oldlevel = tmp->level;
  int oldSize = 1 << (oldlevel + MIN_POWER);
  int newlevel = get_level(size);
  if (oldlevel >= newlevel) {
    return ptr;
  }
  void *newmem;
  size -= sizeof(mall_h_t);
  newmem = malloc(size);
  if (!newmem) {
    return NULL;
  }
  memcpy(newmem, ptr, oldSize - sizeof(mall_h_t));
  free(ptr);
  return newmem;
}
