/* 
 * Queue list defenition and implementation
 */

#ifndef QUEUE_H
#define QUEUE_H

/* List Node */
typedef struct node {
	int process_pid; /* PID */
	int status; /* Process exit status */
	struct timeval start; /* Process start time */
	struct timeval end; /* Process end time */
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
struct node *dequeue(struct queue *queue_list){
	struct node *out_node = NULL;
	if (queue_list == NULL || queue_list->head == NULL)
		return NULL;
	out_node = queue_list->head;
	if (queue_list->head->next != NULL){
		queue_list->head = out_node->next;
	}
	else{ /* queue ended, no more nodes */
		queue_list->head = NULL;
		queue_list->tail = NULL;
	}
	return out_node;
}

/* ----------------------------------------------------------
 * Find a node with given PID
 * Returns pointer to list node if found
 * ---------------------------------------------------------- */
struct node *find_pid(struct queue *queue_list, int child_id){
	struct node *crawler = NULL;
	if (queue_list == NULL)
		return NULL;
	crawler = queue_list->head;
	if (crawler == NULL)
		return NULL;
	if (child_id == crawler->process_pid)
		return crawler;
	while (crawler->next != NULL) {
		crawler = crawler->next;
		if (child_id == crawler->process_pid)
			return crawler;
	}
	return NULL;
}

#endif
