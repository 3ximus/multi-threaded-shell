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

struct node {
	int process_pid;
	struct node *next;
};

struct queue {
	struct node *head;
	struct node *tail;
};

void enqueue(struct queue *queue_list, int pid){
	if (queue_list == NULL)
		return;

	struct node *in_node = (struct node*)malloc(sizeof(struct node));
	in_node->process_pid = pid;

	if (queue_list->head == NULL)
		queue_list->head = in_node;
	if (queue_list->tail == NULL)
		queue_list->tail = in_node;
	else{
		queue_list->tail->next = in_node;
		queue_list->tail = in_node;
	}
	
}

int dequeue(struct queue *queue_list){
	int pid;
	struct node *out_node = NULL;
	if (queue_list == NULL || queue_list->head == NULL)
		return -1;
	out_node = queue_list->head;
	if (queue_list->head->next != NULL){
		queue_list->head = out_node->next;
	}
	else{
		/* queue ended, no more nodes */
		queue_list->head = NULL;
		queue_list->tail = NULL;
	}
	pid = out_node->process_pid;
	free(out_node);
	return pid;
}

int main(int argc, char **argv, char **envp){

	int vector_size = 7; /* program name + 5 arguments */
	char **arg_vector = (char **)malloc(vector_size * sizeof(char *));
	struct queue *process_queue = (struct queue*)malloc(sizeof(struct queue));
	int child_status;
	int child_pid = 0;

	/* initialize child process queue */
	process_queue->head = NULL;
	process_queue->tail = NULL;

	while(1){
		printf("> ");	
		readLineArguments(arg_vector, vector_size);

		if (strcmp(arg_vector[0], "exit") == 0 && arg_vector[1] == NULL){
			/* wait for child process to exit with status 1 */
			while(process_queue->head != NULL)
				printf("Child pid: %d\n", waitpid(dequeue(process_queue), &child_status, WIFEXITED(EXIT_SUCCESS)));
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
				enqueue(process_queue, child_pid);
		}
	}
	return 0;
}