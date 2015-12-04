/* ----------------------------------------------------------
 * Shell Paralela
 * Grupo 82
 * Sistemas Operativos 2015
 * 63572 Pedro Carneiro
 * 76959 Fábio Almeida
 * 79764 César Alcobia
 * ---------------------------------------------------------- */

/* System Includes */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <pthread.h>
#include <time.h>
#include <errno.h>

/* Our Includes */
#include "commandlinereader.h"
#include "list.h"
#include "queue.h"
#include "errorhandling.h"

/* Defines */
#define VECTOR_SIZE		7 /* program name + 5 arguments */
#define MAXPAR			3
#define EXIT_COMMAND	"exit"
#define BUFFER_SIZE		100
#define LOG_TEMP_BUFF	50

#define STDIN			0
#define STDOUT			1
#define STDERR			2
#define PIPENAME "par-shell-in"

int child_count = 0, exit_command = 0, total_execution_time = 0, iteration = 0;
pthread_mutex_t mutex;
pthread_cond_t write_cond;
pthread_cond_t max_par;
pthread_cond_t new_child;
list_t *lst;
queue_l *writing_queue;
FILE* log_fd;

/* Forward declaractions */
void *monitor(void);
void *writer(void);
void read_log(void);

int main(int argc, char **argv){
	int numArgs;
	int in_fd; /* replacement for stdin */
	char buffer[BUFFER_SIZE];
	char *arg_vector[VECTOR_SIZE];
	time_t starttime;
	
	int child_pid;
	pthread_t monitor_thread;
	pthread_t writer_thread;
	lst = lst_new();
	writing_queue = new_queue();

	/* Initialize synchronization objects */
	pthread_mutex_init_(&mutex, NULL);
	pthread_cond_init_(&write_cond, 0);
	pthread_cond_init_(&max_par, NULL);
	pthread_cond_init_(&new_child, NULL);

	if ((log_fd = fopen("./log.txt", "a+")) == NULL){
		perror("[ERROR] opening log file");
		exit(EXIT_FAILURE);
	}

	/* replace stdin */
	mkfifo_(PIPENAME, S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP); /* make named pipe to receive input from */
	in_fd = open_(PIPENAME, O_RDONLY, S_IRUSR); /* create new file for stdin */
	close_(STDIN); /* close stdin to prepare to receive commands through the named pipe */
	dup2(in_fd, STDIN); /* make duplicate of in_fd and assign to lowest numbered unused file descriptor (stdin) */
	close_(in_fd); /* since we now have a copy of this for stdin we dont need the original */

	read_log(); /* assign total time and iteration values for this execution */
	pthread_create_(&monitor_thread, NULL, (void *)&monitor, NULL); /* Create Monitor Thread */  
	pthread_create_(&writer_thread, NULL, (void *)&writer, NULL); /* Create Writer Thread */
 
 	printf("\033[1;32mPar-Shell Connected to %s\033[0m\nRunning...", PIPENAME);
	while (1) {
		numArgs = readLineArguments(arg_vector, VECTOR_SIZE, buffer, BUFFER_SIZE);
		if (numArgs <= 0) continue;

		if (strcmp(arg_vector[0], EXIT_COMMAND) == 0 ) {

			/* signal exit command */
			exit_command = 1;

			/* wait for monitor thread to end */
			pthread_cond_signal_(&new_child); /* must signal to catch exit flag */
			pthread_join_(monitor_thread, NULL);
			pthread_join_(writer_thread, NULL);

			/* print all the elements in the list */
			lst_print(lst);
			
			/* terminate sync objects */
			pthread_mutex_destroy_(&mutex);
			pthread_cond_destroy_(&write_cond);
			pthread_cond_destroy_(&max_par);
			pthread_cond_destroy_(&new_child);
			unlink_(PIPENAME); /* remove named pipe */
			fclose(log_fd);
			lst_destroy(lst);
			exit(EXIT_SUCCESS);
		}

		/* INTERPRET ANY OTHER COMMAND GIVEN AND LAUNCH NEW PROCESS */

		/* while there are child process slots available launch new child process,
		 * else wait here */
		pthread_mutex_lock_(&mutex);
		while (child_count >= MAXPAR) pthread_cond_wait_(&max_par, &mutex);
		pthread_mutex_unlock_(&mutex);

		child_pid = fork();
		if (child_pid < 0){ /* test for error in fork */
			perror("[ERROR] forking process");
			continue;
		}
		if (child_pid == 0){ /* execute on child */
			int new_stdout_fd;
			char child_stdout_pathname[20];
			/* set stdout of child process to be a new file*/
			sprintf(child_stdout_pathname, "par-shell-out-%d.txt", getpid()); /* format filename */
			new_stdout_fd = open_(child_stdout_pathname, O_CREAT|O_RDWR, S_IRUSR); /* create new file for stdout */
			close_(STDOUT); /* close child default stdout */
			dup_(new_stdout_fd); /* make suplicate of new_fd and assign it to lowest numbered unused file descriptor, in this case STDOUT wich has been closed */
			close_(new_stdout_fd); /* close original filedescriptor */
			if (execv(arg_vector[0], arg_vector) == -1) {
				perror("[ERROR] executing program.");
				exit(EXIT_FAILURE);
			}
		}
		else { /* execute on parent */
			/* Main thread
			 *
			 * Child processes are inserted into the process list saving their pid
			 * and their start time.
			 * Child counter is incremented.
			 * The condition variable waiting for a new child process is signaled.
 			 */
			time(&starttime); /* get start time of child process */
			pthread_mutex_lock_(&mutex);
			insert_new_process(lst, child_pid, starttime);
			child_count++;
			pthread_cond_signal_(&new_child);
			pthread_mutex_unlock_(&mutex);
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
		pthread_mutex_lock_(&mutex);
		while (child_count <= 0)pthread_cond_wait_(&new_child, &mutex);
		pthread_mutex_unlock_(&mutex);

		child_pid = wait(&child_status);
		time(&endtime);

		pthread_mutex_lock_(&mutex);
		update_terminated_process(lst, child_pid, endtime, child_status);
		enqueue(writing_queue, child_pid); /* put process on hold to be written */
		--child_count;
		pthread_cond_signal_(&write_cond);
		pthread_cond_signal_(&max_par);
		pthread_mutex_unlock_(&mutex);

		pthread_mutex_lock_(&mutex);
		if (exit_command != 0 && child_count <= 0) {
			pthread_mutex_unlock_(&mutex);
			pthread_exit(NULL);
		}
		pthread_mutex_unlock_(&mutex);
	}
}

/* ----------------------------------------------------------
 * Writer Thread
 *
 * Writes to log file 
 * ---------------------------------------------------------- */
void *writer(void){
	int pid, time_diff, total_execution_time_local, iteration_local;

	while (1){
		pthread_mutex_lock_(&mutex);
		/* wait for a condition */
		while (writing_queue->head == NULL) pthread_cond_wait_(&write_cond, &mutex);
		pid = dequeue(writing_queue); /* remove element to be written */
		time_diff = get_time_diff(lst, pid); /* calculate time execution time */

		/* increment variables */
		total_execution_time += time_diff;
		iteration++;

		/* put variables locally */
		iteration_local = iteration;
		total_execution_time_local = total_execution_time;
		pthread_mutex_unlock_(&mutex);

		/* write to file */
		fprintf(log_fd, "iteracao %d\npid: %d execution time: %d s\ntotal execution time: %d s\n",iteration_local, pid, time_diff, total_execution_time_local);
		fflush(log_fd);

		/* exit thread */
		pthread_mutex_lock_(&mutex);
		if (exit_command != 0 && child_count == 0){
			pthread_mutex_unlock_(&mutex);
			pthread_exit(NULL);		
		}
		pthread_mutex_unlock_(&mutex);
	}
}

/* ----------------------------------------------------------
 * Read Log File
 * Read log file to set total time and iteration values
 * ---------------------------------------------------------- */
void read_log(void){
	char iteration_buff[LOG_TEMP_BUFF];
	char pid_buff[LOG_TEMP_BUFF];
	char time_buff[LOG_TEMP_BUFF];
	
	/* read all the lines, the ones left are the last ones to read */
	while ((fgets(iteration_buff, LOG_TEMP_BUFF, log_fd) != NULL) &&
		   (fgets(pid_buff, LOG_TEMP_BUFF, log_fd) != NULL) &&
		   (fgets(time_buff, LOG_TEMP_BUFF, log_fd) != NULL))
		{continue;}
	/* interpret read buffers */
	sscanf(iteration_buff, "iteracao %d\n", &iteration);
	sscanf(time_buff, "total execution time: %d s\n", &total_execution_time);
}
