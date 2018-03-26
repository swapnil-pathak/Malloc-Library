#include <stdio.h>
#include <math.h>
#include "dsnf.h"

__thread mall_h_t *freeList[MAX_LEVEL + 1];
pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

/*
Makes the sbrk() system call and returns pointer to the address
*/
void *get_from_heap(size_t size) {
  void *ret = sbrk(0);
  if((sbrk(size)) == (void *)-1) {
    errno = ENOMEM;
    return NULL;
  }
  return ret;
}

/*
Main function
*/
void *get_memory(size_t size) {
  if(size <= 0) {
    return NULL;
  }
  pthread_mutex_lock(&lock);  //Big lock inefficient

  size += sizeof(mall_h_t);

  size_t pgsize = get_page_size(size);
  if(pgsize > (MAX_BLOCK_SIZE / 2)) {
    return get_from_mmap(pgsize);
  }

  int level = get_level(size);

  int split_level = level;
  if(freeList[level] == NULL) {
    int i = level + 1;
    while ((i <= MAX_LEVEL) && (freeList[i] == NULL)) {
      i++;
    }

    if (i > MAX_LEVEL) {

      mall_h_t* newHeap = (mall_h_t *) get_from_heap(MAX_BLOCK_SIZE);

      newHeap->status = 0;
      newHeap->level = MAX_LEVEL;
      newHeap->next = NULL;
      newHeap->prev = NULL;
      split_level = MAX_LEVEL;
      freeList[MAX_LEVEL] = newHeap;

    } else {
      split_level = i;

    }
  }

  if(freeList[level] == NULL) {
    split_buddy(freeList, level, split_level);
  }


  if(freeList[level] != NULL) {
    mall_h_t* allocatedblock = freeList[level];
    allocatedblock->status = 1;
    freeList[level] = allocatedblock->next;
    if(freeList[level] != NULL) {
      freeList[level]->prev = NULL;
    }
    allocatedblock->next = NULL;
    allocatedblock->prev = NULL;
    allocatedblock->level = level;
    pthread_mutex_unlock(&lock);
    return (void*)((char*) allocatedblock + sizeof(mall_h_t));
  }
  pthread_mutex_unlock(&lock);
  return NULL;
}
/*
Splits block to buddies
Add to freelist if free
*/
void split_buddy(mall_h_t **freeList, int level, int split_level) {

  int new_level;
  while ((freeList[level] == NULL) && (split_level > level)) {
    new_level = split_level - 1;
    if(freeList[new_level] == NULL) {
      mall_h_t *old = freeList[split_level];
      freeList[split_level] = freeList[split_level]->next;
      if(freeList[split_level]) {
        freeList[split_level]->prev = NULL;
      }
      mall_h_t *left_buddy = old;
      left_buddy->status = 0;
      left_buddy->level = new_level;
      left_buddy->next = NULL;
      left_buddy->prev = NULL;
      int foo = 1 << (new_level + MIN_POWER);
      mall_h_t *right_buddy = (mall_h_t*)((void*)old + foo);
      right_buddy->status = 0;
      right_buddy->level = new_level;
      right_buddy->next = NULL;
      right_buddy->prev = NULL;
      left_buddy->next = right_buddy;
      right_buddy->prev = left_buddy;
      freeList[new_level] = left_buddy;
    }
    split_level--;
  }
}

/*
Returns a multiple of PAGESIZE to check for further allocation
*/
size_t get_page_size(size_t size) {
  size_t pgsize = PAGESIZE;
  size_t rawsize = pgsize - sizeof(mall_h_t);
  while (rawsize < size) {
    pgsize *= 2;
    rawsize = pgsize - sizeof(mall_h_t);
  }
  return pgsize;
}

int get_level(size_t size) {
  int level = 0;
  int mem = 32;
  while(size > mem) {
    mem *= 2;
    level++;
  }
  return level;
}

/*
Handle memory allocation using mmap()
*/
void *get_from_mmap(size_t pgsize) {
  void *map;
  if((map = mmap(0, pgsize, PROT_EXEC | PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_PRIVATE, -1, 0)) == MAP_FAILED) {
    exit(-1);
  }
  mall_h_t *bh = (mall_h_t*)map;
  bh->status = 1;
  bh->level = get_level(pgsize);
  bh->next = NULL;
  bh->prev = NULL;
  pthread_mutex_unlock(&lock);
  return (void *)((char*)bh + sizeof(mall_h_t));
}

/*
Main function free
*/
void free_memory(void* ptr) {
  pthread_mutex_lock(&lock);
  mall_h_t *freeblock = (mall_h_t*)((char*)ptr - sizeof(mall_h_t));
  
  size_t size = 1 << (freeblock->level + MIN_POWER);
  if(size > (MAX_BLOCK_SIZE / 2)){
    munmap((void*)(char*)ptr - sizeof(mall_h_t), size);
    pthread_mutex_unlock(&lock);
    return;
  }

  freeblock->status = 0;

  while(freeblock->level <= MAX_LEVEL) {
    if(freeblock->level == MAX_LEVEL) {
      freeblock->next = freeList[MAX_LEVEL];
      if(freeList[MAX_LEVEL]) {
        freeList[MAX_LEVEL]->prev = freeblock;
      }
      freeblock->prev = NULL;
      freeList[MAX_LEVEL] = freeblock;
      pthread_mutex_unlock(&lock);
      break;
    }
    mall_h_t *found_buddy = find_buddy(freeblock);

    if(found_buddy != NULL && found_buddy->status != 1 && found_buddy->level == freeblock->level) {
      if(found_buddy->prev) {
        found_buddy->prev->next = found_buddy->next;
      }
      if(found_buddy->next) {
        found_buddy->next->prev = found_buddy->prev;
      }
      found_buddy->next = NULL;
      found_buddy->prev = NULL;
      found_buddy->status = 0;
      if((void*)freeblock > (void*)found_buddy) {
        freeblock = found_buddy;
      }
      freeblock->level += 1;
      continue;
    } else {
      freeblock->next = freeList[freeblock->level];
      if(freeList[freeblock->level]) {
        freeList[freeblock->level]->prev = freeblock;
      } 
      freeblock->prev = NULL;
      freeList[freeblock->level] = freeblock;
      pthread_mutex_unlock(&lock);
      break;
    }  
  }
}

mall_h_t *find_buddy(mall_h_t* freeblock) {
  if(freeblock->level == MAX_LEVEL) {
    return NULL;
  } else {
    int foo = 1 << (freeblock->level + MIN_POWER);
    mall_h_t *found_buddy = (mall_h_t *) ((uintptr_t) freeblock ^ foo);
    return found_buddy;
  }
}

/*
mallinfo_t mallinfo() {
  mallinfo_t minfo;
  minfo.ar_sbrk +=;
  minfo.ar_mmap +=;
  minfo.ar_no +=;
  minfo.blk_no +=;
  minfo.used_blks +=;
  minfo.free_blks +=;
  minfo.req_alloc +=;
  minfo.req_free +=;
  return minfo;
}*/


