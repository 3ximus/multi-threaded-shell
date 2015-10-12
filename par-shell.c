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
	int status;
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

struct node *dequeue(struct queue *queue_list){
	struct node *out_node = NULL;
	if (queue_list == NULL || queue_list->head == NULL)
		return NULL;
	out_node = queue_list->head;
	if (queue_list->head->next != NULL){
		queue_list->head = out_node->next;
	}
	else{
		/* queue ended, no more nodes */
		queue_list->head = NULL;
		queue_list->tail = NULL;
	}
	return out_node;
}

struct node *find_pid(struct queue *queue_list, int child_id){
	struct node *crawler = NULL;
	crawler = queue_list->head;

	if (child_id == crawler->process_pid){
		return crawler;
	}

	while (crawler->next != NULL){
		crawler = crawler->next;
		if (child_id == crawler->process_pid){
			return crawler;
		}
	}
	return NULL;
}


int main(int argc, char **argv){
	struct queue *q_list = (struct queue*)malloc(sizeof(struct queue));
	struct node *temp = (struct node *)malloc(sizeof(struct node));
	int vector_size = 7; /* program name + 5 arguments */
	char **arg_vector = (char **)malloc(vector_size * sizeof(char *));
	int child_pid = 0, child_status, child_count = 0;

	while(1){
		if (readLineArguments(arg_vector, vector_size) == -1) {
			perror("[ERROR] Reading command");
			exit(EXIT_FAILURE);
		}	
		if (strcmp(arg_vector[0], "exit") == 0 && arg_vector[1] == NULL){
			/* wait for child process to exit */
			while(child_count > 0){
				child_pid = wait(&child_status);
				find_pid(q_list, child_pid)->status = child_status;
				child_count--;
			}
			while((temp = dequeue(q_list)) != NULL)
				printf("PID: %d, STATUS: %d\n", temp->process_pid, temp->status);
			free(arg_vector);
			free(temp);
			exit(1);
		}
		else{
			child_pid = fork();
			if (child_pid == 0){
				/* execute on child */
				if (execv(arg_vector[0], arg_vector) == -1)
					perror("[ERROR] executing program.");
					exit(EXIT_FAILURE);
			}
			else{
				/* execute on parent */
				enqueue(q_list, child_pid);
				child_count++;
			}
		}
	}
	return 0;
}
