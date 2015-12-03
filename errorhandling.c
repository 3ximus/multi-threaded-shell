/*
 * Sistemas Operativos 2015-2016
 * Grupo 82
 *
 * Error handling.
 */
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include "errorhandling.h"

/* --------------------------------------------------------------------------
 * Threads error checking
 * -------------------------------------------------------------------------- */

/* pthread_create */
void
pthread_create_(pthread_t * thread, const pthread_attr_t * attr,
		void * start_routine, void * arg)
{
	if ((pthread_create(thread, attr, start_routine, arg)) != 0) {
		fprintf(stderr, "[ERROR] Pthread creating error\n");
		exit(EXIT_FAILURE);
	}
}

/* pthread_join */
void
pthread_join_(pthread_t thread, void ** retval)
{
	if ((pthread_join(thread, retval)) != 0) {
		fprintf(stderr, "[ERROR] Pthread joining error\n");
		exit(EXIT_FAILURE);
		}
}

/* --------------------------------------------------------------------------
 * Mutexes error checking
 * -------------------------------------------------------------------------- */

/* pthread_mutex_init */
void
pthread_mutex_init_(pthread_mutex_t * mutex, pthread_mutexattr_t * attr)
{
	if ((pthread_mutex_init(mutex, attr)) != 0) {
		fprintf(stderr, "[ERROR] Mutex initialization error.\n");
		exit(EXIT_FAILURE);
	}
}

/* pthread_mutex_lock */
void
pthread_mutex_lock_(pthread_mutex_t * mutex)
{
	if ((pthread_mutex_lock(mutex)) != 0) {
		fprintf(stderr, "[ERROR] Mutex locking error\n");
		exit(EXIT_FAILURE);
	}
}

/* pthread_mutex_unlock */
void
pthread_mutex_unlock_(pthread_mutex_t * mutex)
{
	if ((pthread_mutex_unlock(mutex)) != 0) {
		fprintf(stderr, "[ERROR] Mutex unlocking error\n");
		exit(EXIT_FAILURE);
	}
}

/* pthread_mutex_destroy */
void
pthread_mutex_destroy_(pthread_mutex_t * mutex)
{
	if ((pthread_mutex_destroy(mutex)) != 0) {
		fprintf(stderr, "[ERROR] Mutex destroying error\n");
		exit(EXIT_FAILURE);
	}
}



/* --------------------------------------------------------------------------
 * Semaphores error checking
 * -------------------------------------------------------------------------- */

/* sem_init */
void
sem_init_(sem_t * sem, int shared, unsigned int value)
{
	if ((sem_init(sem, shared, value)) == -1) {
		perror("[ERROR] Semaphore initialization error.");
		exit(EXIT_FAILURE);
	}
}

/* sem_wait */
void
sem_wait_(sem_t * sem)
{
	if ((sem_wait(sem)) == -1) {
		perror("[ERROR] Semaphore wait error.");
		exit(EXIT_FAILURE);
	}
}

/* sem_post */
void
sem_post_(sem_t * sem)
{
	if ((sem_post(sem)) == -1) {
		perror("[ERROR] Semaphore post error.");
		exit(EXIT_FAILURE);
	}
}

/* sem_destroy */
void
sem_destroy_(sem_t * sem)
{
	if ((sem_destroy(sem)) == -1) {
		perror("[ERROR] Semaphore destroying error.");
		exit(EXIT_FAILURE);
	}
}


/* --------------------------------------------------------------------------
 * Conditions error checking
 * -------------------------------------------------------------------------- */

/* pthread_mutex_init */
void
pthread_cond_init_(pthread_cond_t * cond, const pthread_condattr_t * attr)
{
	if ((pthread_cond_init(cond, attr)) != 0) {
		fprintf(stderr, "[ERROR] cond initialization error.\n");
		exit(EXIT_FAILURE);
	}
}

/* pthread_cond_lock */
void
pthread_cond_wait_(pthread_cond_t * cond, pthread_mutex_t * mutex)
{
	if ((pthread_cond_wait(cond, mutex)) != 0) {
		fprintf(stderr, "[ERROR] cond  wait error\n");
		exit(EXIT_FAILURE);
	}
}

/* pthread_cond_unlock */
void
pthread_cond_signal_(pthread_cond_t * cond)
{
	if ((pthread_cond_signal(cond)) != 0) {
		fprintf(stderr, "[ERROR] cond signal error\n");
		exit(EXIT_FAILURE);
	}
}

/* pthread_cond_destroy */
void
pthread_cond_destroy_(pthread_cond_t * cond)
{
	if ((pthread_cond_destroy(cond)) != 0) {
		fprintf(stderr, "[ERROR] cond destroying error\n");
		exit(EXIT_FAILURE);
	}
}

/* --------------------------------------------------------------------------
 * File operations
 * -------------------------------------------------------------------------- */

/* dup */
int dup_(int in_fd){
	int fd;
	if ((fd = dup(in_fd)) == -1){
		perror("[ERROR] dup error\n");
		exit(EXIT_FAILURE);
	}
	return fd;
}

/* open */
int open_(const char* filename, int oflag, mode_t mode){
	int fd;
	if ((fd = open(filename, oflag, mode)) == -1) {
		perror("[ERROR] open file error\n");
		exit(EXIT_FAILURE);
	}
	return fd;
}

/* close */
int close_(int fd){
	int ret_val;
	if ((ret_val = close(fd)) == -1) {
		perror("[ERROR] closing file error\n");
		exit(EXIT_FAILURE);
	}
	return ret_val;
}

/* mkfifo */
int mkfifo_(char* pathname, mode_t mode){
	int ret_val;
	if ((ret_val = mkfifo(pathname, mode)) == -1){
		perror("[ERROR] mkfifo error\n");
		exit(EXIT_FAILURE);
	}
	return ret_val;
}


/* unlink */
int unlink_(char* pathname){
	int ret_val;
	if ((ret_val = unlink(pathname)) == -1){
		perror("[ERROR] unlink error\n");
		exit(EXIT_FAILURE);
	}
	return ret_val;
}
