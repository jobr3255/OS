#ifndef QUEUE_H
#define QUEUE_H

#include <stdio.h>

#define MAX_NAME_LENGTHS 1025

struct Node {
	char* value;
	struct Node *next;
};

struct Queue {
	struct Node *first;
	struct Node *last;
	unsigned int maxSize;
	unsigned int size;
};

int init(Queue* q, int size);
// int is_empty(Queue* q);
// int is_full(Queue* q);
// int queue_push(Queue* q, void* payload);
// void* queue_pop(Queue* q);
// void queue_cleanup(Queue* q);

#endif
