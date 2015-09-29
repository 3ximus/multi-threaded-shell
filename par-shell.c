//
// Shell Paralela
// Sistemas Operativos 2015
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#include "commandlinereader.h"

int main(int argc, char **argv, char **envp){

	int vector_size = 7; /* program name + 5 arguments */
	char **arg_vector = (char **)malloc(vector_size * sizeof(char *));
	int child_pid = 0, child_status, child_count = 0;

	while(1){
		if (readLineArguments(arg_vector, vector_size) == -1) {
			perror("[ERROR] Reading command");
			exit(EXIT_FAILURE);
		}	
		if (strcmp(arg_vector[0], "exit") == 0 && arg_vector[1] == NULL){
			/* wait for child process to exit with status 1 */
			while(child_count > 0){
				child_pid = wait(&child_status);
				printf("Child PID = %d, Child Status = %d.\n", child_pid, child_status);
				child_count--;
			}
			free(arg_vector);
			exit(1);
		}
		else{
			child_pid = fork();
			if (child_pid == 0){
				/* execute on child */
				if (execve(arg_vector[0], arg_vector, envp) == -1)
					perror("[ERROR] executing program.");
					exit(EXIT_FAILURE);
			}
			else
				/* execute on parent */
				child_count++;
		}
		free(arg_vector);
	}
	return 0;
}
