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
	int numArgs;
	char buffer[BUFFER_SIZE];
	char *arg_vector[VECTOR_SIZE];
	char pipename[PATHNAME_SIZE];
	if (argc != 2) {
		printf("Usage ./par-shell-terminal <named-pipe>");
		exit(EXIT_FAILURE);
	}
	strncpy(pipename, argv[1],PATHNAME_SIZE);

	while (1) {
		numArgs = readLineArguments(arg_vector, VECTOR_SIZE, buffer, BUFFER_SIZE);
		if (numArgs <= 0) continue;
		if (strcmp(arg_vector[0], EXIT_COMMAND) == 0 ) {

		}
		if (strcmp(arg_vector[0], STAT_COMMAND) == 0 ) {

		}
	}
	return 0;
}

