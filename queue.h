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

/* ----------------------------------------------------------
 * Make new queue
 * ---------------------------------------------------------- */
queue_l *new_queue();

/* ----------------------------------------------------------
 * Add a new node to the queue with given pid
 * Other fields Uninitialized
 * ---------------------------------------------------------- */
void enqueue(struct queue *queue_list, int pid);

/* ----------------------------------------------------------
 * Remove element from list, must be freed
 * ---------------------------------------------------------- */
int dequeue(struct queue *queue_list);

#endif
