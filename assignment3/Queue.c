#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#define MAX_NAME_LENGTHS 1025

struct Node {
	char* value;
	struct Node *next;
};

typedef struct Queue {
	struct Node *first;
	struct Node *last;
	unsigned int maxSize;
	unsigned int size;
}Queue;

void init(struct Queue *q, int maxSize) {
    q->first = NULL;
    q->last = NULL;
    q->maxSize = maxSize;
    q->size = 0;
}

char* pop(struct Queue *q) {
    if(q->size > 0){
    	q->size--;
    	struct Node *tmp = q->first;
        char* returnValue = tmp->value;
    	q->first = q->first->next;
    	free(tmp);
        return returnValue;
    }
    printf("\033[91mERROR\033[0m");
    printf(": Queue is empty\n");
    return NULL;
}

bool push(struct Queue *q, char* data) {
    if(q->size < q->maxSize){
    	q->size++;
    	if (q->first == NULL) {
    		q->first = (struct Node *) malloc(sizeof(struct Node));
    		q->first->value = data;
    		q->first->next = NULL;
    		q->last = q->first;
    	} else {
    		q->last->next = (struct Node *) malloc(sizeof(struct Node));
    		q->last->next->value = data;
    		q->last->next->next = NULL;
    		q->last = q->last->next;
    	}
        return true;
    }
    printf("\033[91mERROR\033[0m");
    printf(": Queue is full\n");
    return false;
}

bool is_empty(Queue* q){
    if(q->size <= 0){
		return true;
    }
    else{
		return false;
    }
}

bool is_full(Queue* q){
    if(q->size >= q->maxSize){
		return true;
    }
    else{
		return false;
    }
}

void cleanup(Queue* q)
{
    while(!is_empty(q)){
		pop(q);
    }
}
