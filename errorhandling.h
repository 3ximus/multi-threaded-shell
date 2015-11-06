#ifndef ERRORHANDLING_H
#define ERRORHANDLING_H

#include <pthread.h>
#include <semaphore.h>

/* --------------------------------------------------------------------------
 * Threads error checking
 * -------------------------------------------------------------------------- */

/* pthread_create */
void pthread_create_(pthread_t * thread, const pthread_attr_t * attr,
		void * start_routine, void * arg);

/* pthread_join */
void pthread_join_(pthread_t thread, void ** retval);

/* --------------------------------------------------------------------------
 * Mutexes error checking
 * -------------------------------------------------------------------------- */

/* pthread_mutex_init */
void
pthread_mutex_init_(pthread_mutex_t * mutex, pthread_mutexattr_t * attr);

/* pthread_mutex_lock */
void pthread_mutex_lock_(pthread_mutex_t * mutex);

/* pthread_mutex_unlock */
void pthread_mutex_unlock_(pthread_mutex_t * mutex);

/* pthread_mutex_destroy */
void pthread_mutex_destroy_(pthread_mutex_t * mutex);



/* --------------------------------------------------------------------------
 * Semaphores error checking
 * -------------------------------------------------------------------------- */

/* sem_init */
void sem_init_(sem_t * sem, int shared, unsigned int value);

/* sem_wait */
void sem_wait_(sem_t * sem);

/* sem_post */
void sem_post_(sem_t * sem);

/* sem_destroy */
void sem_destroy_(sem_t * sem);

#endif
