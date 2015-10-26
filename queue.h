/* Queue list defenition */

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