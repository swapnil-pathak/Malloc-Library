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

mall_h_t* get_buddy(mall_h_t* nodeToFree);
arena_h_t * get_arena_by_num(int arena_req);


void free(void *ptr){

  char buf[1024];
 
  if( ptr == NULL ){
    return ;
  }

  mall_h_t *nodeToFree = (mall_h_t*) ( (void *) ptr - sizeof(mall_h_t) );
 
  if ( nodeToFree > arena_to_thread->endAddress || nodeToFree < arena_to_thread->startAddress ){
    nodeToFree = NULL;
    return ;
  }

  if( nodeToFree->status != 1 ){

    return ;
  }

  if( nodeToFree->size == 0 ){

    return ;
  }

  pthread_mutex_lock(&mutex);
  if(!arena_to_thread){
    pthread_mutex_unlock(&mutex);
    return;
  }
  arena_h_t *arena = arena_to_thread; 
  pthread_mutex_unlock(&mutex);

  pthread_mutex_lock(&arena->arena_lock);

  nodeToFree->status = 0;
  mall_h_t *buddy = get_buddy(nodeToFree);
  mall_h_t *tempNext, *tempPrev;
  while( buddy != NULL ){
 
    int currentIndex = LOG(buddy->size) - 1;
      if( buddy->next != NULL && buddy->prev != NULL ){
      tempNext = buddy->next;
      tempPrev = buddy->prev;
      tempPrev->next = tempNext;
      tempNext->prev = tempPrev;
     
    }else if( buddy->next != NULL && buddy->prev == NULL ){
      tempNext = buddy->next;
      tempNext->prev = NULL;

  

      arena->freeList[currentIndex] = tempNext;
    }else if( buddy->next == NULL && buddy->prev != NULL ){
      tempPrev = buddy->prev;
      tempPrev->next = NULL;
    
    }else if( buddy->next == NULL && buddy->prev == NULL ){
      arena->freeList[currentIndex] = NULL;
    
      buddy->next = NULL;
    }else{
   
    }
    nodeToFree->next = NULL;
    if( buddy < nodeToFree ){
      nodeToFree = buddy;
    }
    buddy = NULL;
    nodeToFree->size = nodeToFree->size * 2;
//    fflush(stdout);
    buddy = get_buddy(nodeToFree);
  }
  
  int free_index = LOG(nodeToFree->size) - 1;
  if( free_index >= 12 ){
  
    free_index = 12;
  }
  if( arena->freeList[free_index] != NULL ){
    mall_h_t *curr = arena->freeList[free_index];
    while( curr->next != NULL ){
      curr = curr->next;
    }
    curr->next = nodeToFree;
    nodeToFree->prev = curr;
  }else{
    arena->freeList[free_index] = nodeToFree;
  }
  if( free_index < 12 ){
    arena_to_thread->smblks += 1;     
  }
  arena_to_thread->hblkhd -= nodeToFree->size;    
  arena_to_thread->uordblks -= nodeToFree->size;  
  arena_to_thread->fordblks -= nodeToFree->size;  

  pthread_mutex_unlock(&arena->arena_lock);
}

mall_h_t* get_buddy(mall_h_t* nodeToFree){
  int freeListIndex = LOG(nodeToFree->size) - 1;
  int arena_num = nodeToFree->arena_num;
  pthread_mutex_lock(&mutex);
  arena_h_t * arena = arena_to_thread; 
  pthread_mutex_unlock(&mutex);

  char buf[1024];
  

  mall_h_t *curr = arena->freeList[freeListIndex];
  if ( curr > arena_to_thread->endAddress || curr < arena_to_thread->startAddress ){
    curr = NULL;
    return NULL;
  }

  while( curr != NULL ){

    write(STDOUT_FILENO, buf, strlen(buf) + 1);

    if(curr->status==0){
   
      if( (nodeToFree->blockMaxAddr == curr->blockMaxAddr))
        {
       
        return curr;
      }

    }
    curr = curr->next;
  }
  return curr;
}

arena_h_t * get_arena_by_num(int arena_req){
  if( arena_head == NULL ){
    return NULL;
  }
  arena_h_t *curr = arena_head;
  int cnt = 0;
  while(cnt < arena_req){
    cnt++;
    curr = curr->nextArena;
  
  }
  return curr;
}
