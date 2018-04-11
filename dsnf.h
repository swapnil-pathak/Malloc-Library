#include <stdio.h>
#include <pthread.h>

#ifndef __DSNF_H__
#define __DSNF_H__

#define BASE 2

#define LOG(size) (ceil((log(size) / log(BASE))))
#define PG_SIZE 4096
#define FREE_LOG(PG_SIZE)


typedef struct MallocHeader{
	size_t size;
	struct node *next;
	struct node *prev;
	int status;
	int *blockMaxAddr;
	int *blockMinAddr;
	int arena_num;
	int dummy;
} mall_h_t;

typedef struct ArenaHeader{
	pthread_mutex_t arena_lock;
	mall_h_t *freeList[13];
	int lists_in_arena;
	void *startAddress;
	void *endAddress;
	void *nextArena;
	void *prevArena;
	size_t size;
	int ordblks;   
	int smblks;    
	int hblks;     
	int hblkhd;    
	int uordblks;  
  int fordblks;
} arena_h_t;

typedef struct mallinfo {

	int arena;     
	int ordblks;   
  int smblks;    
  int hblks;     
  int hblkhd;    
  int usmblks;   
  int fsmblks;   
  int uordblks;  
  int fordblks;  
  int keepcost;  
};

extern __thread mall_h_t *head;
extern arena_h_t *arena_head;
extern __thread arena_h_t *arena_to_thread;
extern pthread_mutex_t mutex;
extern int num_threads;

extern int lists_in_arena;

#endif
