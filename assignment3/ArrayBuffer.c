#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

#define MAXSIZE 25

typedef struct Node{
    void* value;
} Node;

typedef struct ArrayBuffer{
    Node* array;
    int front;
    int rear;
    int maxSize;
} ArrayBuffer;

int queue_init(ArrayBuffer* q, int size){
    if(size>0) {
        q->maxSize = size;
    } else {
        q->maxSize = MAXSIZE;
    }
    // allocate memory for node in array
    q->array = malloc(sizeof(Node) * (q->maxSize));
    if(!(q->array)){
        perror("Error on queue Malloc");
        return false;
    }
    // set all array values to NULL
    for(int i=0; i < q->maxSize; ++i){
        q->array[i].value = NULL;
    }
    q->front = 0;
    q->rear = 0;
    return q->maxSize;
}

int queue_is_empty(ArrayBuffer* q){
    if((q->front == q->rear) && (q->array[q->front].value == NULL)){
        return 1;
    }
    else{
        return 0;
    }
}

int queue_is_full(ArrayBuffer* q){
    if((q->front == q->rear) && (q->array[q->front].value != NULL)){
        return 1;
    }
	return 0;
}

void* queue_pop(ArrayBuffer* q){
    void* ret_value;
    if(queue_is_empty(q)){
	       return NULL;
    }
    ret_value = q->array[q->front].value;
    q->array[q->front].value = NULL;
    q->front = ((q->front + 1) % q->maxSize);
    return ret_value;
}

int queue_push(ArrayBuffer* q, void* new_value){
    if(queue_is_full(q)){
        return false;
    }
    q->array[q->rear].value = new_value;
    q->rear = ((q->rear+1) % q->maxSize);
    return true;
}

// free all allocated memory
void queue_cleanup(ArrayBuffer* q)
{
    while(!queue_is_empty(q)){
        queue_pop(q);
    }
    free(q->array);
}
