
/* 
 * Mutex Usage
 */

#ifndef MUTEX_H
#define MUTEX_H

pthread_mutex_t mutex_data;

/* -----------------------------------------------------------
 * Error checked Mutex lock
 * ----------------------------------------------------------- */
void mutex_lock(void) {
  if(pthread_mutex_lock(&mutex_data) != 0)
  {
    perror("[ERROR] Locking mutex");
    exit(EXIT_FAILURE);
  }
}

/* -----------------------------------------------------------
 * Error checked Mutex unlock
 * ----------------------------------------------------------- */
void mutex_unlock(void) {
  if(pthread_mutex_unlock(&mutex_data) != 0)
  {
    perror("[ERROR] Unlocking mutex");
    exit(EXIT_FAILURE);
  }
}

#endif