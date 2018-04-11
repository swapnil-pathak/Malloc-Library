#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <unistd.h>
#include <math.h>
#include <sys/mman.h>
#include <errno.h>
#include "dsnf.h"

int posix_memalign(void **memptr, size_t alignment, size_t size){
  if( size == (size_t) 0 ){
    *memptr = NULL;
    return 0;
  }else if(alignment % 2 != 0 || alignment % sizeof( void*) != 0 ){
    return EINVAL;
  }

  *memptr = malloc(size);
  if(*memptr == NULL){
    return ENOMEM;
  }else{
    return 0;
  }
}
