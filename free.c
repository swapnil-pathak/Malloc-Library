#include <stdio.h>
#include "dsnf.h"

void free(void *ptr) {
  if(ptr == NULL) {
      return;
  }
  free_memory(ptr);
  return;
}
