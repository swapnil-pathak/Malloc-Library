#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <unistd.h>
#include <math.h>
#include <sys/mman.h>
#include <errno.h>
#include <pthread.h>
#include "dsnf.h"

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
int lists_in_arena = 13;

__thread mall_h_t *head = NULL;
arena_h_t *arena_head = NULL;
int num_threads = 0;

__thread arena_h_t* arena_to_thread = NULL;

arena_h_t *initArenas();
arena_h_t * get_arena(int arena_req);
mall_h_t *get_memory_arena(arena_h_t *assign_arena, size_t size);
mall_h_t *free_in_arena(arena_h_t* freeList, size_t size);
mall_h_t *get_mem_arena(arena_h_t* freeList, size_t size);

void * malloc(size_t size){

  if(size == 0){
    return NULL;
  }
  char buf[1024];

  size_t allocSize = size + sizeof(mall_h_t);
  if(arena_head == NULL){
    initArenas();
  }

  int free_index = LOG(allocSize) - 1;

  int cores = sysconf(_SC_NPROCESSORS_ONLN);
  int arena_req = num_threads % cores;
  arena_h_t *assign_arena;
  if ( arena_to_thread == NULL ) {
    pthread_mutex_lock(&mutex);
    assign_arena = get_arena(num_threads);
    arena_to_thread = assign_arena;
    pthread_mutex_unlock(&mutex);
  }else{
    assign_arena = arena_to_thread;
  }

  pthread_mutex_lock(&assign_arena->arena_lock);
  mall_h_t *alloc_buddy = get_memory_arena(assign_arena, size);
  if( alloc_buddy == NULL ) {
    errno = ENOMEM;
    pthread_mutex_unlock(&mutex);
    pthread_mutex_unlock(&assign_arena->arena_lock);
    return NULL;
  }else{
    alloc_buddy->status = 1;
    pthread_mutex_unlock(&assign_arena->arena_lock);
    return (char *) alloc_buddy + sizeof(mall_h_t);
  }
}

arena_h_t * initArenas(){
  char buf[1024];
  long long int arena_size = PG_SIZE * PG_SIZE * 25;
  int cores = sysconf(_SC_NPROCESSORS_ONLN); 
  int cnt = 0;
  arena_h_t *curr = arena_head;
  arena_h_t *prev = arena_head;
  void *temp;
  int end_addr_heap = (int)sbrk(0);
  end_addr_heap = end_addr_heap + arena_size; 
  temp = mmap(NULL, arena_size, PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
  if (temp == NULL) {
    sprintf(buf, "%s","mmap failed" );
    write(STDOUT_FILENO, buf, strlen(buf) + 1);
    return;
  }
  curr = (arena_h_t*)temp;
  curr->size = arena_size;
  curr->nextArena = NULL;
  curr->prevArena = NULL; 
  curr->startAddress = temp;
  curr->endAddress = (void* ) (temp  + arena_size);

  int free_buddy_size =  arena_size - sizeof(arena_h_t);

  mall_h_t *free_buddy = (mall_h_t*) ( (char *) curr + sizeof(arena_h_t) );
  free_buddy->size = free_buddy_size;
  free_buddy->next = NULL;
  free_buddy->prev = NULL;
  free_buddy->status = 0;
  free_buddy->arena_num = cnt;
  free_buddy->blockMaxAddr = (int *) ( (char*) curr + arena_size );
  curr->freeList[12] = free_buddy;
  curr->ordblks = 1;
  curr->smblks = 0;
  curr->hblkhd = 0;
  curr->hblkhd = 0;
  curr->uordblks = 0;
  curr->fordblks = arena_size;

  if( arena_head == NULL ){
    arena_head = curr;
  }

  cnt++;
  prev = curr;
 
  return curr;
}


mall_h_t *get_memory_arena(arena_h_t *assign_arena, size_t size){
  if( size == 0 ){
    return NULL;
  }
  size_t allocSize = size + sizeof(mall_h_t);
  int free_index = (ceil((log(allocSize) / log(BASE)))) - 1;

  char buf[1024];
 
  if (free_index < 12){
    mall_h_t* temp = free_in_arena(assign_arena, allocSize);
    arena_to_thread->smblks -= 1;
    arena_to_thread->hblkhd += size;
    arena_to_thread->uordblks += size;
    arena_to_thread->fordblks -= size;
    return temp + sizeof(mall_h_t);
  }else{
    mall_h_t* temp = get_mem_arena(assign_arena, allocSize);
    arena_to_thread->ordblks -= 1;
    arena_to_thread->hblkhd += size;
    arena_to_thread->uordblks += size;
    arena_to_thread->fordblks -= size;
    return temp + sizeof(mall_h_t);
  }

}


mall_h_t *free_in_arena(arena_h_t* arena, size_t size){
  int i = 0;
  char buff2[1024];

  int free_index = (ceil((log(size) / log(BASE)))) - 1;
  mall_h_t *nodeToAllocate = NULL;

  if( arena->freeList[free_index] != NULL ){
    
    nodeToAllocate = (mall_h_t *) arena->freeList[free_index];

    nodeToAllocate->next = NULL;
    nodeToAllocate->prev = NULL;
    nodeToAllocate->status = 1;
    nodeToAllocate->size = size - sizeof(mall_h_t);

    if(nodeToAllocate->next != NULL){
      arena->freeList[free_index] = arena->freeList[free_index]->next;
      arena->freeList[free_index]->prev = NULL;
    }else{
      arena->freeList[free_index] = NULL;
    }

    return nodeToAllocate;
  }else{
   
    char buf[1024];
    for(i=free_index;i<lists_in_arena-1;i++){
      mall_h_t *temp;
      int currentIndex = i;

      
      if( arena->freeList[i] != NULL ){
     
        while( currentIndex != free_index ){
          temp = arena->freeList[currentIndex];
          temp->size = temp->size / 2;
          mall_h_t *new = (mall_h_t*) ( (char*) temp + temp->size );

          new->size = temp->size;
          new->status = 0;
          new->prev = temp;
          new->next = NULL;
          new->blockMaxAddr = temp->blockMaxAddr;
          new->dummy = temp->dummy;
          new->arena_num = temp->arena_num;
          if( temp->next != NULL ){
            mall_h_t *next = temp->next;
            if( !next->prev ){
              next = NULL;
            }else{
              next->prev = NULL;
              arena->freeList[currentIndex] = next;
            }

          }else{
            arena->freeList[currentIndex] = NULL;
          }
          temp->next = new;  
          currentIndex--;
          arena->freeList[currentIndex] = temp;
          arena_to_thread->smblks += 1;
        }
        nodeToAllocate = arena->freeList[currentIndex];
        if(nodeToAllocate->next != NULL){
          arena->freeList[currentIndex] = arena->freeList[currentIndex]->next;
          arena->freeList[currentIndex]->prev = NULL;
          nodeToAllocate->next = NULL;
          nodeToAllocate->prev = NULL;
        }else{
          arena->freeList[currentIndex] = NULL;
        }
        return nodeToAllocate;
      }else if( arena->freeList[i] == NULL && currentIndex == lists_in_arena-2 ){
        
        mall_h_t *moreMemory = get_mem_arena(arena, PG_SIZE); // sbrk(PG_SIZE);

        if(moreMemory == NULL){
          errno = ENOMEM;
          return NULL;
        }
        arena->freeList[lists_in_arena-2] = (mall_h_t *) moreMemory;
        arena->freeList[lists_in_arena-2]->size = PG_SIZE;
        arena->freeList[lists_in_arena-2]->status = 0;
        arena->freeList[lists_in_arena-2]->next = NULL;
        arena->freeList[lists_in_arena-2]->prev = NULL;
        arena->freeList[lists_in_arena-2]->dummy = 99999;
        arena->freeList[lists_in_arena-2]->blockMaxAddr = (int *) ( (char*) moreMemory + PG_SIZE );
        arena->freeList[lists_in_arena-2]->blockMinAddr = arena->startAddress;
        arena->freeList[lists_in_arena-2]->arena_num = moreMemory->arena_num;
      
        i--;
      }
    }

  }
  return nodeToAllocate;
}

mall_h_t * get_mem_arena(arena_h_t* arena, size_t size){
  mall_h_t *freeMemory = arena->freeList[lists_in_arena-1];
  mall_h_t *temp = freeMemory;
  char buf[1024];
 
  while( temp->next != NULL && temp->status != 0 ){
    temp = temp->next;
  }
  if( temp == NULL ){
  
    return NULL; 
  }
 
  mall_h_t *newFreeBlock = (mall_h_t*) ( (char *) temp + size );
  newFreeBlock->size = temp->size - size;

  newFreeBlock->prev = temp->prev;
  newFreeBlock->next = temp->next;

  arena->freeList[lists_in_arena-1] = (mall_h_t *) newFreeBlock;
  arena->freeList[lists_in_arena-1]->status = 0;
  arena->freeList[lists_in_arena-1]->dummy = 99999;
  arena->freeList[lists_in_arena-1]->blockMaxAddr = freeMemory->blockMaxAddr;
  arena->freeList[lists_in_arena-1]->blockMinAddr = freeMemory->blockMinAddr;

  temp->prev = NULL;
  temp->next = NULL;
  temp->size = 4096;
  temp->dummy = 99999;
  temp->blockMaxAddr = (void *) ( (char*) temp + PG_SIZE ) ;
  temp->blockMinAddr = freeMemory->blockMinAddr;
  return temp;
}

arena_h_t * get_arena(int arena_req){
  if( arena_head == NULL ){
    arena_h_t *newArena = initArenas();
    arena_to_thread = newArena;
    return initArenas();
  }
  int cores = sysconf(_SC_NPROCESSORS_ONLN);
  arena_h_t* arenaToReturn;
  if(num_threads > cores){
    int arena_num = num_threads % cores;
    int t = 0;
    arena_h_t *curr = arena_head;
    while(t<arena_num){
      t++;
      curr = curr->nextArena;
    }
    arenaToReturn = curr;
  }else{
    arena_h_t *curr = arena_head;
    int cnt = 0;
    while(curr->nextArena != NULL){
      cnt++;
      curr = curr->nextArena;
     
    }
    char buf[1024];
   
    arena_h_t *newArena = initArenas();
    curr->nextArena = newArena;
    newArena->prevArena = curr;
    arenaToReturn = newArena;

  }


  return arenaToReturn;
}
