
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
#include <semaphore.h>
#include <errno.h>

/* Our Includes */
#include "commandlinereader.h"
#include "queue.h"
#include "err_handling.h"

/* Defines */
#define VECTOR_SIZE 7 /* program name + 5 arguments */
#define MAXPAR 4
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
  int numArgs;
  char *arg_vector[VECTOR_SIZE];
  time_t start_time;
	
	int child_pid;
	pthread_t monitor_thread;
	q_list = new_queue();

	/* Initialize synchronization objects */
  pthread_mutex_init_(&mutex, NULL);
  sem_init_(&maxChilds, 0, MAXPAR); /* semaphore initialized to MAXPAR */
  sem_init_(&noChilds, 0, 0);

	/* Create Thread */
  pthread_create_(&monitor_thread, NULL, (void *) monitor, NULL);
	while (1) {
		numArgs = readLineArguments(arg_vector, VECTOR_SIZE, buffer, BUFFER_SIZE);
		if (numArgs <= 0) {
			continue;
    }
		if (strcmp(arg_vector[0], EXIT_COMMAND) == 0 ) {

			/* signal exit command */
			exit_command = 1;

			/* wait for monitor thread to end */
      sem_post_(&noChilds);
			pthread_join_(monitor_thread, NULL);

      /* print all the elements in the list and free their memory */
			while ((temp = dequeue(q_list)) != NULL) {
				printf("PID: %d, STATUS: %d, DURATION: %ld SECONDS\n", temp->process_pid,
						temp->status, temp->end - temp->start);
				free(temp);
			}
			
			/* terminate sync objects */
			pthread_mutex_destroy_(&mutex);
			sem_destroy_(&maxChilds);
      sem_destroy_(&noChilds);

			free(q_list);
			exit(EXIT_SUCCESS);
		}

    /* while there are child process slots available launch new child process,
     * else wait here */
    sem_wait_(&maxChilds);

    child_pid = fork(); // testar erro no fork
    if (child_pid == 0){ /* execute on child */
      if (execv(arg_vector[0], arg_vector) == -1) {
        perror("[ERROR] executing program.");
        exit(EXIT_FAILURE);
          }
    }
    else if (child_pid == -1) {
      perror("[ERROR] fork error.");
      continue;
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
      time(&start_time); /* get start time of child process */
      pthread_mutex_lock_(&mutex);
      enqueue(q_list, child_pid, start_time);
      child_count++;
      pthread_mutex_unlock_(&mutex);
      sem_post_(&noChilds);
    }
	}
}

/* ----------------------------------------------------------
 * Monitor Thread
 *
 * Monitor thread waits for child processes to end and
 * saves their status and end time.
 * If there are no children, monitor thread is suspended waiting for any new
 * child process.
 * ---------------------------------------------------------- */
int monitor(void) {
	int child_pid;
	int child_status;
	node_l * temp = NULL;

	while (1) {
    sem_wait_(&noChilds); /* wait for a new child process */


    pthread_mutex_lock_(&mutex);
    if (child_count > 0) {
      pthread_mutex_unlock_(&mutex);

      child_pid = wait(&child_status);

      pthread_mutex_lock_(&mutex);
      temp = find_pid(q_list, child_pid);
      temp->status = child_status;
      time(&temp->end);
      --child_count;
      pthread_mutex_unlock_(&mutex);

      sem_post_(&maxChilds);
    }
    else if (exit_command != 0) {
      pthread_mutex_unlock_(&mutex);
			pthread_exit(NULL);
    }
	}
}
