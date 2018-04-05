#ifndef DSNF_H
#define DSNF_H

#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <assert.h>
#include <errno.h>
#include <pthread.h>
#include <math.h>
#include <stdint.h>

//For each block
typedef struct MallocHeader {
  uint8_t status;
  uint8_t level;  
  struct MallocHeader *next;
  struct MallocHeader *prev;
}mall_h_t;

//For each arena
typedef struct ArenaHeader {
  pthread_mutex_t arenalock;
  mall_h_t *freeList[7];
  void *startAddr;
  void *endAddr;
  void *nextArena;
  void *prevArena;
} arena_h_t;

//Malloc stats
typedef struct mallinfo {
  int ar_sbrk;
  int ar_mmap;
  int ar_no;
  int blk_no;
  int used_blks;
  int free_blks;
  int req_alloc;
  int req_free;
} mallinfo_t;


#define PAGESIZE sysconf(_SC_PAGESIZE) //Pagesize needed
#define MIN_LEVEL 0
#define MAX_LEVEL 6
#define MIN_POWER 5
#define MAX_POWER 12
#define MIN_BLOCK_SIZE 32
#define MAX_BLOCK_SIZE PAGESIZE

extern arena_h_t *arena_head;

void *get_from_heap(size_t size);
void *get_memory(size_t size);
void split_buddy(mall_h_t **freeList, int level, int split_level);
size_t get_page_size(size_t size);
int get_level(size_t size);
void *get_from_mmap(size_t pageSize);
void free_memory(void* ptr);
mall_h_t *find_buddy(mall_h_t* releasedBlock);
void *realloc(void *ptr, size_t size);
void *calloc(size_t nmemb, size_t size);
void free(void *ptr);
void *malloc(size_t size);
void *reallocarray(void *ptr, size_t nmemb, size_t size);
void *memalign(size_t alignment, size_t size);
int posix_memalign(void **memptr, size_t alignment, size_t size);

mallinfo_t mallinfo(void);
void malloc_stats();
void print();
#endif
