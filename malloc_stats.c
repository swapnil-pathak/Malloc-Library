#include "dsnf.h"
#include <string.h>
#include <stdio.h>
#include <unistd.h>

void malloc_stats() {
 // pthread_mutex_lock(&lock);
  char buf[4096];
    
  snprintf(buf, 4096, "Here"); 
  write(STDOUT_FILENO, buf , strlen(buf)+1); 
  //pthread_mutex_unlock(&lock);
}
