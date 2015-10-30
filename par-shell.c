
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
#include "err_handling.h"

/* Defines */
#define VECTOR_SIZE 7 /* program name + 5 arguments */
#define MAXPAR 2
#define EXIT_COMMAND "exit"
#define BUFFER_SIZE 100

int child_count = 0;
int exit_command = 0;
pthread_mutex_t mutex;
sem_t maxChilds;
sem_t noChilds;
queue_l *q_list;

/* Forward declaraction */
int monitor(void);

int main(int argc, char **argv){
	node_l *temp = NULL;
  char buffer[BUFFER_SIZE];
	
	int child_pid;
	q_list = (queue_l*)malloc(sizeof(queue_l));
	pthread_t monitor_thread;

	/* Initialize synchronization objects */
  pthread_mutex_init_(&mutex, NULL);
  sem_init_(&maxChilds, 0, MAXPAR); /* semaphore initialized to MAXPAR */
  sem_init_(&noChilds, 0, 0);

	/* Create Thread */
  pthread_create_(&monitor_thread, NULL, (void *) monitor, NULL);

	while (1) {
		int numArgs;
		char **arg_vector = (char **)malloc(VECTOR_SIZE * sizeof(char *));
		numArgs = readLineArguments(arg_vector, VECTOR_SIZE, buffer, BUFFER_SIZE);
		if (numArgs == 0) {
      free(arg_vector);
			continue;
    }
		if (strcmp(arg_vector[0], EXIT_COMMAND) == 0 && arg_vector[1] == NULL) {

			/* signal exit command */
			exit_command = 1;

			/* wait for monitor thread to end */
      sem_post_(&noChilds);
			pthread_join_(monitor_thread, NULL);

			/* while ((temp = dequeue(q_list)) != NULL) {
				printf("PID: %d, STATUS: %d, DURATION: %ld SECONDS\n", temp->process_pid,
						temp->status, temp->end.tv_sec - temp->start.tv_sec);
				free(temp);
			} */
			
			/* terminate sync objects */
			pthread_mutex_destroy_(&mutex);
			sem_destroy_(&maxChilds);
      sem_destroy_(&noChilds);

			free(q_list);
			free(arg_vector);
			exit(EXIT_SUCCESS);
		}
		else {
      sem_wait_(&maxChilds);

			child_pid = fork();
			if (child_pid == 0){ /* execute on child */
				if (execv(arg_vector[0], arg_vector) == -1) {
					perror("[ERROR] executing program.");
					exit(EXIT_FAILURE);
        		}
        free(arg_vector);
        exit(EXIT_SUCCESS);
			}
			else { /* execute on parent */
        /*
         * Main thread
         *
         * Child processes are registered into the queue saving their pid
         * and their start time.
         * Child counter is incremented.
         * The semaphore controlling maximum parallel children is decremented.
         */
				pthread_mutex_lock_(&mutex);
				enqueue(q_list, child_pid);
				gettimeofday(&(find_pid(q_list, child_pid)->start), NULL);
				child_count++;
        if (child_count == 1) {
          sem_post_(&noChilds);
        }
				pthread_mutex_unlock_(&mutex);
				
			}
		}
		free(arg_vector);
	}
	// free(q_list);
	// return 0;
}

/* ----------------------------------------------------------
 * Monitor Thread
 *
 * Monitor thread waits for child processes to end if there are any
 * ---------------------------------------------------------- */
int monitor(void) {
	int child_pid;
	int child_status;
	node_l * temp = NULL;

	while (1) {
    pthread_mutex_lock_(&mutex);
    if (child_count > 0) {
      child_pid = wait(&child_status);
      temp = find_pid(q_list, child_pid);
      temp->status = child_status;
      gettimeofday(&(temp->end), NULL);
      --child_count;
      pthread_mutex_unlock_(&mutex);
      sem_post_(&maxChilds);
    }
    else if (exit_command != 0) {
      pthread_mutex_unlock_(&mutex);
			pthread_exit(NULL);
		}
    else {
      pthread_mutex_unlock_(&mutex);
      sem_wait_(&noChilds);
    }
	}
}
