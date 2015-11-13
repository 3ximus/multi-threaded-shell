/* 
 * Queue list defenition and implementation
 */

#ifndef QUEUE_H
#define QUEUE_H

/* List Node */
typedef struct node {
	int process_pid; /* PID */
	struct node *next; /* Pointer to next node */
}node_l;

/* Queue (FIFO) */
typedef struct queue {
	struct node *head; /* List top */
	struct node *tail; /* List bottom */
}queue_l;

queue_l *new_queue(){
	queue_l *queue;
	queue = (queue_l*)malloc(sizeof(queue_l));
	queue->head = NULL;
	queue->tail = NULL;
	return queue;
}

/* ----------------------------------------------------------
 * Add a new node to the queue with given pid
 * Other fields Uninitialized
 * ---------------------------------------------------------- */
void enqueue(struct queue *queue_list, int pid){
	if (queue_list == NULL)
		return;
	struct node *in_node = (struct node*)malloc(sizeof(struct node));
	in_node->process_pid = pid;
	in_node->next = NULL;

	if (queue_list->head == NULL)
		queue_list->head = in_node;
	if (queue_list->tail == NULL)
		queue_list->tail = in_node;
	else{
		queue_list->tail->next = in_node;
		queue_list->tail = in_node;
	}
}

/* ----------------------------------------------------------
 * Remove element from list, must be freed
 * ---------------------------------------------------------- */
int dequeue(struct queue *queue_list){
	int pid;
	struct node *out_node = NULL;
	if (queue_list == NULL || queue_list->head == NULL)
		return -1;
	out_node = queue_list->head;
	if (queue_list->head->next != NULL){
		queue_list->head = out_node->next;
	}
	else{ /* queue ended, no more nodes */
		queue_list->head = NULL;
		queue_list->tail = NULL;
	}
	pid = out_node->process_pid;
	free(out_node);
	return pid;
}

#endif
