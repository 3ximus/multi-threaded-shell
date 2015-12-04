/*
 * Sistemas Operativos 2015-2016
 * Grupo 82
 *
 * Terminal Shell Paralela
 */

/* System Includes */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>

/* Our Includes */
#include "errorhandling.h"

 /* Defines */
#define VECTOR_SIZE			7 /* program name + 5 arguments */
#define PATHNAME_SIZE 		20
#define EXIT_COMMAND 		"exit\n"
#define STATS_COMMAND 		"stats\n"
#define EXIT_GLOBAL_COMMAND	"exit-global\n"
#define BUFFER_SIZE			100
#define STDIN				0
#define STDOUT				1
#define STDERR				2
#define STATS_BUFFER_SIZE	40
 
int main(int argc, char** argv){
	int pipe_fd;
	size_t size = 0;
	ssize_t nbytes;
	char *buffer;
	char pid_buffer[20];
	char pipename[PATHNAME_SIZE];
	char stats_buffer[STATS_BUFFER_SIZE];

	if (argc != 2) { /* read arguments */
		printf("Usage ./par-shell-terminal <named-pipe>");
		exit(EXIT_FAILURE);
	}

	strncpy(pipename, argv[1],PATHNAME_SIZE);
	
	/* try to open FIFO, if unexistent par-shell-terminal will exit with error */
	pipe_fd = open_(pipename, O_RDWR, S_IWUSR|S_IRUSR);

	/* notify par-shell that a new terminal has been opened */
	sprintf(pid_buffer, "__new_term__ %d\n", getpid());
	write_(pipe_fd, pid_buffer, strlen(pid_buffer));

	while ((nbytes = getline(&buffer, &size, stdin)) > 0) {
		if (strcmp(buffer, EXIT_COMMAND) == 0 ) {
			/* notify par-shell we are exiting */
			sprintf(pid_buffer, "__close_term__ %d\n", getpid());
			write_(pipe_fd, pid_buffer, strlen(pid_buffer));
			free(buffer);
			close_(pipe_fd);
			printf("Exiting...\n");
			exit(EXIT_SUCCESS);
		}
		if (strcmp(buffer,  STATS_COMMAND) == 0 ) {
			int term_id, child_count, total_execution_time;
			char *cwd = getcwd(NULL, 0);
			sprintf(buffer, "stats %d\n", getpid()); /* send stats command and pid through the pipe so par-shell returns her stats */
			write_(pipe_fd, buffer, strlen(buffer)); /* writes buffer into the pipe */
			sleep(1); /* wait for par-shell to respond */
			while(read_(pipe_fd, stats_buffer, STATS_BUFFER_SIZE) == 0);
			sscanf(stats_buffer, "__stats__ %d child %d total %d",
					&term_id, &child_count, &total_execution_time);
			if (term_id == getpid())
				printf("Stats:\tPID: %d Path: %s\n\tParShell:\tCurrent running processes: %d\n\t\t\tTotal execution time: %d\n", getpid(), cwd, child_count, total_execution_time);
			else printf("[ERROR] Something went wrong with your request. Try again");
			free(cwd);
			continue;
		}
		if (strcmp(buffer, EXIT_GLOBAL_COMMAND) == 0 ) {
			strcpy(buffer, "exit\n"); /* puts the exit command on the buffer so par-shell exits */
			printf("Terminating par-shell...\n"); /* writes buffer into the pipe */
			write_(pipe_fd, buffer, strlen(buffer));
		}
		/* everything else is sent to the pipe to be interpreted as a command by the par-shell */
		write_(pipe_fd, buffer, strlen(buffer));
	}
}

