/*
 * list.h - definitions and declarations of the integer list 
 */

#ifndef LIST_H
#define LIST_H

#include <stdlib.h>
#include <stdio.h>
#include <time.h>



/* lst_item - each element of the list points to the next element */
typedef struct lst_item {
   int pid;
   time_t starttime;
   time_t endtime;
   int status;
   struct lst_item *next;
} lst_item_t;

/* list_t */
typedef struct {
   lst_item_t * first;
} list_t;



/* lst_new - allocates memory for list_t and initializes it */
list_t* lst_new();

/* lst_destroy - free memory of list_t and all its items */
void lst_destroy(list_t *);

/* insert_new_process - insert a new item with process id and its start time in list 'list' */
void insert_new_process(list_t *list, int pid, time_t starttime);

/* update_terminated_process - updates endtime of element with pid 'pid' */
void update_terminated_process(list_t *list, int pid, time_t endtime, int status);

/* lst_print - print the content of list 'list' to standard output */
void lst_print(list_t *list);

/* get_time_diff - get time diference between start and end of a given pid */
int get_time_diff(list_t *list, int pid);

#endif

