
/* ----------------------------------------------------------
 * Shell Paralela
 * Grupo 82
 * Sistemas Operativos 2015
 ---------------------------------------------------------- */

/* System Includes */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <pthread.h>
#include <time.h>
#include <sys/time.h>
#include <semaphore.h>
#include <errno.h>

/* Our Includes */
#include "commandlinereader.h"
#include "queue.h"
#include "mutex.h"

/* Defines */
#define VECTOR_SIZE 7 /* program name + 5 arguments */
#define MAXPAR 2

int child_count = 0;
int exit_command = 0;
sem_t activeChilds;
sem_t noChilds;
sem_t maxChilds;
queue_l *q_list;

/* Forward declaraction */
int monitor();

int main(int argc, char **argv){
	node_l *temp = NULL;
	
	int child_pid;
	q_list = (queue_l*)malloc(sizeof(queue_l));
	pthread_t monitor_thread_ID;

	/* Initialize synchronization objects */
	if ((pthread_mutex_init(&mutex_data, NULL)) != 0) {
		perror("[ERROR] pthread_mutex_init");
		exit(EXIT_FAILURE);
	}
	if ((sem_init(&activeChilds, 0, 0)) == -1) {
		perror("[ERROR] sem_init");
		exit(EXIT_FAILURE);
	}
	if ((sem_init(&noChilds, 0, 0)) == -1) {
		perror("[ERROR] sem_init");
		exit(EXIT_FAILURE);
	}
	if ((sem_init(&maxChilds, 0, 0)) == -1) {
		perror("[ERROR] sem_init");
		exit(EXIT_FAILURE);
	}
	
	/* Create Thread */
	if (pthread_create(&monitor_thread_ID, NULL, (void *)monitor, NULL) != 0){
		perror("[ERROR] creating thread");
		exit(EXIT_FAILURE);
	}

	while(1){
		int numArgs;
		char **arg_vector = (char **)malloc(VECTOR_SIZE * sizeof(char *));
		numArgs = readLineArguments(arg_vector, VECTOR_SIZE);
		if (numArgs == 0)
			continue;

		if (strcmp(arg_vector[0], "exit") == 0 && arg_vector[1] == NULL) {
			/* signal exit command */
			exit_command = 1;
			/* wait for monitor thread to end */
			if ((sem_wait(&activeChilds)) == -1) {
				perror("[ERROR] sem_wait : ");
				exit(EXIT_FAILURE);
			}
			if ((pthread_join(monitor_thread_ID, NULL)) != 0) {
				perror("[ERROR] pthread_join : ");
				exit(EXIT_FAILURE);
			}

			while ((temp = dequeue(q_list)) != NULL) {
				printf("PID: %d, STATUS: %d, DURATION: %ld SECONDS\n", temp->process_pid,
						temp->status, temp->end.tv_sec - temp->start.tv_sec);
				free(temp);
			}
			
			/* terminate sync objects */
			pthread_mutex_destroy(&mutex_data);
			sem_destroy(&activeChilds);
			sem_destroy(&noChilds);
			sem_destroy(&maxChilds);

			free(q_list);
			free(arg_vector);
			exit(EXIT_SUCCESS);
		}
		else {
			mutex_lock();
			if (child_count >= MAXPAR) {
				mutex_unlock();
				sem_wait(&maxChilds);
			}
			mutex_unlock();
			
			child_pid = fork();
			if (child_pid == 0){ /* execute on child */
				if (execv(arg_vector[0], arg_vector) == -1) {
					perror("[ERROR] executing program.");
					exit(EXIT_FAILURE);
        		}
			}
			else { /* execute on parent */
				/* save child process pid in the queue along with starttime
				 * and increment child counter */
				mutex_lock();
				enqueue(q_list, child_pid);
				gettimeofday(&(find_pid(q_list, child_pid)->start), NULL);
				child_count++;
				sem_post(&noChilds);
				mutex_unlock();
				
			}
		}
		free(arg_vector);
	}
	free(q_list);
	return 0;
}

/* ----------------------------------------------------------
 * Monitor Thread
 * ---------------------------------------------------------- */
int monitor() {
	int child_pid;
	int child_status;
	node_l * temp = NULL;

	while (1) {
    if (child_count > 0) {
      mutex_lock();
      child_pid = wait(&child_status);
      temp = find_pid(q_list, child_pid);
      temp->status = child_status;
      gettimeofday(&(temp->end), NULL);
      --child_count;
      if (child_count < MAXPAR) {
        sem_post(&maxChilds);
      }
      mutex_unlock();
      sem_wait(&noChilds);
    }
    else if (exit_command != 0) {
			sem_post(&activeChilds);
			pthread_exit(NULL);
		}
	}
}
