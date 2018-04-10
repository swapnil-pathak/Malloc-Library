#include "dsnf.h"

struct mallinfo mallinfo(){
  struct mallinfo info;
  info.ordblks = 0;
  info.smblks = 0;     
  info.hblkhd = 0;    
  info.uordblks = 0;  
  info.fordblks = 0;

  arena_h_t * curr = arena_head;
  while( curr !=  NULL ) {
    info.ordblks += curr->ordblks;
    info.smblks += curr->smblks;
    info.hblkhd += curr->hblkhd;
    info.uordblks += curr->uordblks;
    info.fordblks += curr->fordblks;
  }

  return info;
}
