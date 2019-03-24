#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

typedef struct FileNode{
    FILE* file;
    int index;
    bool done;
} FileNode;

typedef struct FileBuffer{
    FileNode* array;
    int finishedFiles;
    int index;
    int size;
} FileBuffer;

int file_buffer_init(FileBuffer* buffer, int size){
    if(size>0) {
        buffer->size = size;
    } else {
        perror("Error durring file buffer init. Size is less than 1.\n");
        return 0;
    }
    buffer->array = malloc(sizeof(FileNode) * size);
    if(!(buffer->array)){
        perror("Error allocating file buffer init\n");
        return 0;
    }
    buffer->finishedFiles = 0;
    buffer->index = 0;
    return 1;
}

void fileBufferPush(FileBuffer* buffer, FILE* file){
    buffer->array[buffer->index].file = file;
    buffer->array[buffer->index].index = buffer->index;
    buffer->array[buffer->index].done = false;
    buffer->index++;
}

FileNode* getFileNode(FileBuffer* buffer, int index){
    return &buffer->array[index];
}

FILE* getFile(FileNode* node){
    if(node != NULL)
        return node->file;
    return NULL;
}

FileNode* getNextFileNode(FileBuffer* buffer, FileNode* node){
    int nextIndex = node->index + 1;
    // printf("size: %d\n", buffer->size);
    for(int i=0; i < buffer->size; i++){
        // printf("here %d\n", nextIndex);
        if(nextIndex >= buffer->size){
            nextIndex = nextIndex - buffer->size;
        }
        // printf("here %d\n", nextIndex);
        if(!buffer->array[nextIndex].done){
            // printf("found file %d\n", nextIndex);
            return &buffer->array[nextIndex];
        }
        nextIndex++;
    }
    // printf("no file found\n");
    return NULL;
}

void finishFile(FileBuffer* buffer, FileNode* node){
    if(!node->done){
        node->done = true;
        buffer->finishedFiles++;
    }else{
        // printf("node is already done\n");
    }
}

bool filesFinished(FileBuffer* buffer){
    // printf("finishedFiles = %d size = %d\n", buffer->finishedFiles, buffer->size);
    return buffer->finishedFiles >= buffer->size;
}

void closeFileBuffer(FileBuffer* buffer){
    for(int i=0; i<buffer->size; i++){
        fclose(buffer->array[i].file);
    }
    free(buffer->array);
}
