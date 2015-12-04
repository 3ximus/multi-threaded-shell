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
#define STATS_COMMAND 	"stats"
#define OPEN_T_COMMAND 	"__new_term__"
#define CLOSE_T_COMMAND 	"__close_term__"
#define BUFFER_SIZE		100
#define LOG_TEMP_BUFF	50
#define MAX_TERMINAL	20
#define STATS_BUFFER_SIZE	40
#define STDIN			0
#define STDOUT			1
#define STDERR			2
#define PIPENAME		"par-shell-in"

int child_count = 0, exit_command = 0, total_execution_time = 0, iteration = 0;
int open_terminals[MAX_TERMINAL];
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
void sigint_handler(int x);
void open_terminal(int term_id, int *open_terminals);
void close_terminal(int term_id, int *open_terminals);

int main(int argc, char **argv){
	int numArgs;
	int pipe_fd; /* replacement for stdin */
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
	mkfifo_(PIPENAME, S_IRUSR|S_IWUSR); /* make named pipe to receive input from */
	pipe_fd = open_(PIPENAME, O_RDWR, S_IRUSR|S_IWUSR); /* create new file for stdin */
	close_(STDIN); /* close stdin to prepare to receive commands through the named pipe */
	dup2(pipe_fd, STDIN); /* make duplicate of pipe_fd and assign to lowest numbered unused file descriptor (stdin) */
	/*close_(pipe_fd);  we now have a copy of this for stdin we dont need the original */

	/* Initialize signal handler */
	if (signal(SIGINT, sigint_handler) == SIG_ERR) perror("[ERROR] Couldn't catch SIGINT");

	read_log(); /* assign total time and iteration values for this execution */
	pthread_create_(&monitor_thread, NULL, (void *)&monitor, NULL); /* Create Monitor Thread */  
	pthread_create_(&writer_thread, NULL, (void *)&writer, NULL); /* Create Writer Thread */
 
 	printf("\033[1;32mPar-Shell Connected to %s\033[0m\nRunning...\n", PIPENAME);
	while (1) {
		numArgs = readLineArguments(arg_vector, VECTOR_SIZE, buffer, BUFFER_SIZE);
		if (numArgs <= 0) continue;
		if (strcmp(arg_vector[0], OPEN_T_COMMAND) == 0 ) {
			int term_id = atoi(arg_vector[1]);	
			open_terminal(term_id, open_terminals);
			printf("\033[1;33m[NOTIFY]\033[0m New Terminal open with pid %d.\n", term_id);
			continue;
		}
		if (strcmp(arg_vector[0], CLOSE_T_COMMAND) == 0 ) {
			int term_id = atoi(arg_vector[1]);	
			close_terminal(term_id, open_terminals);
			printf("\033[1;33m[NOTIFY]\033[0m Terminal with pid %d closed.\n", term_id);
			continue;
		}
		if (strcmp(arg_vector[0], STATS_COMMAND) == 0 ) {
			int term_id = atoi(arg_vector[1]);	
			char write_buffer[STATS_BUFFER_SIZE];
			pthread_mutex_lock_(&mutex);
			sprintf(write_buffer, "__stats__ %d child %d total %d", term_id, child_count, total_execution_time);
			pthread_mutex_unlock_(&mutex);
			printf("\033[1;33m[NOTIFY]\033[0m Stats request from %d.\n", term_id);
			write_(pipe_fd, write_buffer, STATS_BUFFER_SIZE);
			sleep(1); /* wait for terminal to read */
			continue;
		}
		printf("Reading %s\n", arg_vector[0]);

		if (strcmp(arg_vector[0], EXIT_COMMAND) == 0 ) {

			/* signal exit command */
			exit_command = 1;

			/* wait for monitor thread to end */
			pthread_cond_signal_(&new_child); /* must signal to catch exit flag */
			pthread_join_(monitor_thread, NULL);
			pthread_cond_signal_(&write_cond); /* must signal to catch exit flag */
			pthread_join_(writer_thread, NULL);

			/* print all the elements in the list */
			lst_print(lst);
			
			/* terminate sync objects */
			pthread_mutex_destroy_(&mutex);
			pthread_cond_destroy_(&write_cond);
			pthread_cond_destroy_(&max_par);
			pthread_cond_destroy_(&new_child);
			unlink_(PIPENAME); /* remove named pipe */

			/* terminate other processes connected to the pipe (par-shell-terminal) */
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
			close(pipe_fd); /* since this is only used on parent lets close it on the child */
			close_(STDOUT); /* close child default stdout */
			dup_(new_stdout_fd); /* make duplicate of new_fd and assign it to lowest numbered unused file descriptor, in this case STDOUT wich has been closed */
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


/* ----------------------------------------------------------
 * Handle the signal SIGINT
 * Causes all other par-shell-terminal to terminate abruptly
 * ---------------------------------------------------------- */
void sigint_handler(int x){
	printf("\033[1;31mReceived SIGINT. Killing all par-shell-terminal processes.\033[0m\n");
	fflush(stdout);
	// KILL ALL PROCESSES
	for (int i = 0; i < MAX_TERMINAL; i++)
		if (open_terminals[i] != 0) {
			kill(open_terminals[i], SIGINT);
			open_terminals[i] = 0;
		}
	unlink_(PIPENAME); /* TO REMOVE */
	exit(EXIT_FAILURE); /* TO REMOVE */
}


/* ----------------------------------------------------------
 * Marks a new terminal as open on the list
 * ---------------------------------------------------------- */
void open_terminal(int term_id, int *open_terminals){
	for (int i = 0; i < MAX_TERMINAL; i++) {
		if (open_terminals[i] == 0){
			open_terminals[i] = term_id;
			break;
		}
	}
}

/* ----------------------------------------------------------
 * Marks a new terminal as closed on the list
 * ---------------------------------------------------------- */
void close_terminal(int term_id, int *open_terminals){
	for (int i = 0; i < MAX_TERMINAL; i++) {
		if (open_terminals[i] == term_id){
			open_terminals[i] = 0;
			break;
		}
	}
}

