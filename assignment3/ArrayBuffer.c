#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

#define MAXSIZE 25

typedef struct Node{
    void* value;
} Node;

typedef struct ArrayBuffer{
    Node* array;
    int front; // index of front of array
    int rear; // index of end of array
    int maxSize;
    bool finishedReading; // keeps track if the requesters are done reading
} ArrayBuffer;

int buffer_init(ArrayBuffer* buffer, int size){
    buffer->finishedReading = false;
    if(size>0) {
        buffer->maxSize = size;
    } else {
        buffer->maxSize = MAXSIZE;
    }
    // allocate memory for node in array
    buffer->array = malloc(sizeof(Node) * (buffer->maxSize));
    if(!(buffer->array)){
        perror("Error durring buffer init");
        return false;
    }
    // set all array values to NULL
    for(int i=0; i < buffer->maxSize; ++i){
        buffer->array[i].value = NULL;
    }
    buffer->front = 0;
    buffer->rear = 0;
    return buffer->maxSize;
}

int buffer_is_empty(ArrayBuffer* buffer){
    if((buffer->front == buffer->rear) && (buffer->array[buffer->front].value == NULL)){
        return 1;
    }
    else{
        return 0;
    }
}

int buffer_is_full(ArrayBuffer* buffer){
    if((buffer->front == buffer->rear) && (buffer->array[buffer->front].value != NULL)){
        return 1;
    }
	return 0;
}

void* buffer_pop(ArrayBuffer* buffer){
    void* ret_value;
    if(buffer_is_empty(buffer)){
        return NULL;
    }
    ret_value = buffer->array[buffer->front].value;
    buffer->array[buffer->front].value = NULL;
    buffer->front = ((buffer->front + 1) % buffer->maxSize);
    return ret_value;
}

// push value into buffer if not full
int buffer_push(ArrayBuffer* buffer, void* new_value){
    if(buffer_is_full(buffer)){
        return false;
    }
    buffer->array[buffer->rear].value = new_value;
    buffer->rear = ((buffer->rear+1) % buffer->maxSize);
    return true;
}

// free all allocated memory
void buffer_cleanup(ArrayBuffer* buffer)
{
    while(!buffer_is_empty(buffer)){
        buffer_pop(buffer);
    }
    free(buffer->array);
}

int isFinished(ArrayBuffer* buffer){
    return buffer->finishedReading;
}

void finished(ArrayBuffer* buffer){
    buffer->finishedReading = true;
}
