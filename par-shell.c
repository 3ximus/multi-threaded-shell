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
#include <pthread.h>
#include <time.h>
#include <sys/time.h>
#include <semaphore.h>
#include <errno.h>

#include "commandlinereader.h"

#define VECTOR_SIZE 7 /* program name + 5 arguments */

/* Data Structures and variables */
struct node {
	int process_pid;
	int status;
	struct timeval start;
	struct timeval end;
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
	
	if (queue_list == NULL) {
		return NULL;
	}
	crawler = queue_list->head;

	if (child_id == crawler->process_pid){
		return crawler;
	}
	while (crawler->next != NULL) {
		crawler = crawler->next;
		if (child_id == crawler->process_pid){
			return crawler;
		}
	}
	return NULL;
}


extern int errno;

int child_count = 0;
int exit_command = 0;
pthread_mutex_t mutExcSem; /* mutual exclusion semaphore */
sem_t activeChilds;
struct queue *q_list;

/* Forward declaractions */
int monitor();

int main(int argc, char **argv){
	struct node *temp = NULL;
	
	int child_pid;
	q_list = (struct queue*)malloc(sizeof(struct queue));
	pthread_t monitor_thread_ID;

	/* Initialize synchronization objects */
	if ((pthread_mutex_init(&mutExcSem, NULL)) != 0) {
		perror("[ERROR] pthread_mutex_init : ");
		exit(EXIT_FAILURE);
	}
	if ((sem_init(&activeChilds, 0, 0)) == -1) {
		perror("[ERROR] sem_init : ");
		exit(EXIT_FAILURE);
	}
	
	/* Create Thread */
	if (pthread_create(&monitor_thread_ID, NULL, (void *)monitor, NULL) != 0){
		exit(EXIT_FAILURE);
	}

	while(1){
		char **arg_vector = (char **)malloc(VECTOR_SIZE * sizeof(char *));
		if (readLineArguments(arg_vector, VECTOR_SIZE) == -1) {
			perror("[ERROR] Reading command");
			exit(EXIT_FAILURE);
		}	
		if (strcmp(arg_vector[0], "exit") == 0 && arg_vector[1] == NULL) {

			/* signal exit command */
			exit_command = 1;
			/* wait for monitor thread to end */
			if ((sem_wait(&activeChilds)) == -1) {
				perror("[ERROR] sem_wait : ");
				exit(EXIT_FAILURE);
			}
			if ((pthread_join(monitor_thread_ID, NULL)) != 0) {
				perror("[ERROR] pthread_join : ");
				exit(EXIT_FAILURE);
			}

			while ((temp = dequeue(q_list)) != NULL) {
				printf("PID: %d, STATUS: %d, DURATION: %ld\n", temp->process_pid,
						temp->status, temp->end.tv_sec - temp->start.tv_sec);
				free(temp);
			}
			
			/* terminate sync objects */
			pthread_mutex_destroy(&mutExcSem);
			sem_destroy(&activeChilds);

			free(q_list);
			free(arg_vector);
			exit(EXIT_SUCCESS);
		}
		else {
			child_pid = fork();
			if (child_pid == 0){ /* execute on child */
				if (execv(arg_vector[0], arg_vector) == -1)
					perror("[ERROR] executing program.");
					exit(EXIT_FAILURE);
			}
			else { /* execute on parent */
				/* save child process pid in the queue along with starttime
				 * and increment child counter */
				pthread_mutex_lock(&mutExcSem);
				enqueue(q_list, child_pid);
				gettimeofday(&(find_pid(q_list, child_pid)->start), NULL);
				child_count++;
				pthread_mutex_unlock(&mutExcSem);
				
			}
		}
		free(arg_vector);
	}
	free(q_list);
	return 0;
}

int monitor() {
	int child_pid;
	int child_status;
	struct node * temp = NULL;

	while (1) {
		if (child_count > 0) {
			pthread_mutex_lock(&mutExcSem);
			child_pid = wait(&child_status);
			temp = find_pid(q_list, child_pid);
			temp->status = child_status;
			gettimeofday(&(temp->end), NULL);
			--child_count;
			pthread_mutex_unlock(&mutExcSem);
		}
		else if (exit_command != 0) {
			sem_post(&activeChilds);
			pthread_exit(NULL);
		}
		else {
			sleep(1);
		}
	}
}
