
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
#include "list.h"
#include "errorhandling.h"

/* Defines */
#define VECTOR_SIZE		7 /* program name + 5 arguments */
#define MAXPAR			4
#define EXIT_COMMAND	"exit"
#define BUFFER_SIZE		100

int child_count = 0;
int exit_command = 0;
pthread_mutex_t mutex;
pthread_cond_t cond;
sem_t maxChilds;
sem_t noChilds;
list_t *lst;
FILE* log_fd;

/* Forward declaractions */
void *monitor(void);
void *writer(void);

int main(int argc, char **argv){
	char buffer[BUFFER_SIZE];
	int numArgs;
	char *arg_vector[VECTOR_SIZE];
	time_t starttime;
	
	int child_pid;
	pthread_t monitor_thread;
	pthread_t writer_thread;
	lst = lst_new();

	/* Initialize synchronization objects */
	pthread_mutex_init_(&mutex, NULL);
	sem_init_(&maxChilds, 0, MAXPAR); /* semaphore initialized to MAXPAR */
	sem_init_(&noChilds, 0, 0);

	if ((log_fd = fopen("./log.txt", 'a+')) == NULL){
		perror("[ERROR] opening log file");
		exit(EXIT_FAILURE);
	}

	/* Create Monitor Thread */
	pthread_create_(&monitor_thread, NULL, (void *)&monitor, NULL);

	/* Create Writer Thread */
	pthread_create_(&writer_thread, NULL, (void *)&writer, NULL);

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
			pthread_join_(writer_thread, NULL);

			/* print all the elements in the list */
			lst_print(lst);
			
			/* terminate sync objects */
			pthread_mutex_destroy_(&mutex);
			sem_destroy_(&maxChilds);
			sem_destroy_(&noChilds);
			fclose(log_fd);
			lst_destroy(lst);
			exit(EXIT_SUCCESS);
		}

		/* while there are child process slots available launch new child process,
		* else wait here */
		sem_wait_(&maxChilds);

		child_pid = fork();
		if (child_pid < 0){ /* test for error in fork */
			perror("[ERROR] forking process");
			continue;
		}
		if (child_pid == 0){ /* execute on child */
			if (execv(arg_vector[0], arg_vector) == -1) {
			perror("[ERROR] executing program.");
			exit(EXIT_FAILURE);
			}
		}
		else { /* execute on parent */
			/* Main thread
			 *
			 * Child processes are registered into the queue saving their pid
			 * and their start time.
			 * Child counter is incremented.
			 * The semaphore controlling maximum parallel children is decremented.
 			 */
			time(&starttime); /* get start time of child process */
			pthread_mutex_lock_(&mutex);
			insert_new_process(lst, child_pid, starttime);
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
void *monitor(void) {
	int child_pid;
	int child_status;
	time_t endtime;

	while (1) {
		sem_wait_(&noChilds); /* wait for a new child process */


		pthread_mutex_lock_(&mutex);
		if (child_count > 0) {
			pthread_mutex_unlock_(&mutex);

			child_pid = wait(&child_status);
			time(&endtime);

			pthread_mutex_lock_(&mutex);
			update_terminated_process(lst, child_pid, endtime, child_status);
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

/* ----------------------------------------------------------
 * Writer Thread
 *
 * "Description"
 * ---------------------------------------------------------- */
void *writer(void){
	static int total_execution_time = 0, iteration = 0;


	int test_pid = 10, test_exec_time = 10;

	while (1){
		/* TODO read iteration and total execution time */

		/* TODO read times */

		/* wait for a condition */
		fprintf(log_fd, "iteration %d\npid: %d execution time: %d s\ntotal execution time: %d s\n",
		 iteration, test_pid, test_exec_time,  total_execution_time);
		fflush(log_fd);


		/* exit thread */
		pthread_mutex_lock_(&mutex);
		if (exit_command != 0){
			pthread_mutex_unlock_(&mutex);
			pthred_exit(NULL);		
		}
	}

}
