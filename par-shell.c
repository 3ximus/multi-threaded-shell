
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
#include "err_check.h"

/* Defines */
#define VECTOR_SIZE 7 /* program name + 5 arguments */
#define MAXPAR 2

int child_count = 0;
int exit_command = 0;
pthread_mutex_t mutex;
sem_t newChild;
sem_t maxChilds;
queue_l *q_list;

/* Forward declaraction */
int monitor(void);

int main(int argc, char **argv){
	node_l *temp = NULL;
	
	int child_pid;
	q_list = (queue_l*)malloc(sizeof(queue_l));
	pthread_t monitor_thread;

	/* Initialize synchronization objects */
  pthread_mutex_init_(&mutex, NULL);
  sem_init_(&newChild, 0, 0);
  sem_init_(&maxChilds, 0, 0);

	/* Create Thread */
  pthread_create_(&monitor_thread, NULL, (void *) monitor, NULL);

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
      pthread_join_(monitor_thread, NULL);

			while ((temp = dequeue(q_list)) != NULL) {
				printf("PID: %d, STATUS: %d, DURATION: %ld SECONDS\n", temp->process_pid,
						temp->status, temp->end.tv_sec - temp->start.tv_sec);
				free(temp);
			}
			
			/* terminate sync objects */
			pthread_mutex_destroy_(&mutex_data);
			sem_destroy_(&newChild);
			sem_destroy_(&maxChilds);

			free(q_list);
			free(arg_vector);
			exit(EXIT_SUCCESS);
		}
		else {
			pthread_mutex_lock_(&mutex);
			if (child_count >= MAXPAR) {
				pthread_mutex_unlock_(&mutex);

        /* maximum running children reached */
				sem_wait(&maxChilds);
			}
			pthread_mutex_unlock_(&mutex);
			
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
				pthread_mutex_lock_(&mutex);
				enqueue(q_list, child_pid);
				gettimeofday(&(find_pid(q_list, child_pid)->start), NULL);
				child_count++;
				sem_post(&newChild);
				pthread_mutex_unlock_(&mutex);
				
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
int monitor(void) {
	int child_pid;
	int child_status;
	node_l * temp = NULL;

	while (1) {
    pthread_mutex_lock_(&mutex);
    if (child_count > 0) {

      /* wait for child process to exit, save its status and endtime
       * and decrement child counter */
      child_pid = wait(&child_status);
      temp = find_pid(q_list, child_pid);
      temp->status = child_status;
      gettimeofday(&(temp->end), NULL);
      --child_count;

      if (child_count < MAXPAR) {
        /* signal a slot for new child if one is waiting */
        sem_post(&maxChilds);
      }
    }
    else if (exit_command != 0) {
			// sem_post(&activeChilds);
			pthread_exit(NULL);
		}
    else {
      sem_wait(&newChild);
    }
  pthread_mutex_unlock_(&mutex);
	}
}
