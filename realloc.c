#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <unistd.h>
#include <math.h>
#include <sys/mman.h>
#include <errno.h>
#include "dsnf.h"

#define BASE 2
#define LOG(size) (ceil((log(size) / log(BASE))))
#define PG_SIZE 4096

void *realloc(void *ptr, size_t size){
  if(ptr == NULL){
    return malloc(size);
  }

  mall_h_t *originalBlock = ptr - sizeof(mall_h_t);
  
  size_t sizeOfOriginalBlock = originalBlock->size;
  size_t sizeToCopy = size > sizeOfOriginalBlock ? sizeOfOriginalBlock : size;

  void *blockToReturn = malloc(size);

  if( sizeToCopy == size){
    blockToReturn = memcpy(blockToReturn, ptr, size);
  }else{
    blockToReturn = memcpy(blockToReturn, ptr, sizeToCopy);
  }
 
  free(ptr);

  return blockToReturn + sizeof(blockToReturn);
}
