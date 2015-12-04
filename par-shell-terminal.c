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
#include "commandlinereader.h"
#include "errorhandling.h"

 /* Defines */
#define VECTOR_SIZE		7 /* program name + 5 arguments */
#define PATHNAME_SIZE 20
#define EXIT_COMMAND "exit"
#define STAT_COMMAND "stats"
#define BUFFER_SIZE		100
 
int main(int argc, char** argv){
	int pipe_fd;
	size_t size = 0;
	ssize_t nbytes;
	char *buffer;
	char pipename[PATHNAME_SIZE];

	if (argc != 2) { /* read arguments */
		printf("Usage ./par-shell-terminal <named-pipe>");
		exit(EXIT_FAILURE);
	}

	strncpy(pipename, argv[1],PATHNAME_SIZE);
	
	/* try to open FIFO, if unexistent par-shell-terminal will exit with error */
	pipe_fd = open_(pipename, O_WRONLY, S_IWUSR);

	while ((nbytes = getline(&buffer, &size, stdin)) > 0) {
		if (strcmp(buffer, EXIT_COMMAND) == 0 ) {
			close_(pipe_fd);
			exit(EXIT_SUCCESS);
		}
		if (strcmp(buffer,  STAT_COMMAND) == 0 ) {

		}
		
		/* INTERPRET ANY OTHER COMMAND GIVEN AND SEND IT TO THE FIFO TO BE READ BY PAR-SHELL */

		while (write_(pipe_fd, buffer, strlen(buffer)) != nbytes){
			fprintf(stderr, "[ERROR] writing to FIFO\n");
			exit(EXIT_FAILURE);
		}
	}
	free(buffer);
	return 0;
}

