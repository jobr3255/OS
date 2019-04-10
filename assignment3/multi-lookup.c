
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <pthread.h>
#include <unistd.h>
#include <stdbool.h>
#include "util.c"
#include "ArrayBuffer.c"
#include "FileBuffer.c"
#include <sys/types.h>
#include <sys/syscall.h>
#include <time.h>

#define USAGE "multi-lookup <# requester> <# resolver> <requester log> <resolver log> [ <data file> ...]"
#define MINARGS 6

#define SBUFSIZE 1025
#define INPUTFS "%1024s"
#define MAX_INPUT_FILES 10
#define MAX_RESOLVER_THREADS 10
#define MAX_REQUESTER_THREADS 8
#define MAX_NAME_LENGTHS 1025
#define MAX_IP_LENGTH INET6_ADDRSTRLEN

typedef struct ThreadInfo{
    int filesServiced;
    int tid;
}ThreadInfo;

typedef struct InputFiles{
    FILE* fileArray;
    int index;
}InputFiles;

//Threads
struct Thread{
	ArrayBuffer* sharedBuffer;
    FileBuffer* sharedInputFiles;
    FileNode* currentFile;
    FILE* readFile;
    FILE* logFile;
    pthread_mutex_t* buffmutex;
    pthread_mutex_t* outmutex;
};

void* request(void* data){
    // holds thread data
	struct Thread* thread = data;
   	char hostname[MAX_NAME_LENGTHS];
	char *domain;
    // flag to check if domain name has been successfully pushed into buffer
	bool done = false;
    FileBuffer* sharedInputFiles = thread->sharedInputFiles;
    FileNode* currentFileNode = thread->currentFile;
	FILE* requestLog = thread->logFile;
	FILE* inputfp;
	pthread_mutex_t* buffmutex = thread->buffmutex;
	ArrayBuffer* sharedBuffer = thread->sharedBuffer;
    int filesServiced = 0;
    pid_t tid = syscall(SYS_gettid);
    while(!filesFinished(sharedInputFiles)){
        inputfp = getFile(currentFileNode);
       	while(fscanf(inputfp, INPUTFS, hostname) > 0){
    		while(!done){
    			domain = malloc(SBUFSIZE);
    			strncpy(domain, hostname, SBUFSIZE);
    			pthread_mutex_lock(buffmutex);
    			while(buffer_is_full(sharedBuffer)){
					pthread_mutex_unlock(buffmutex);
					usleep(rand() % 100 + 5);
					pthread_mutex_lock(buffmutex);
    			}
            	buffer_push(sharedBuffer, domain);
    			pthread_mutex_unlock(buffmutex);
    			done = true;
        	}
    		done = false;
    	}
        filesServiced++;
        printf("\nThread %d finished servicing file %d.\n\n", tid, currentFileNode->index);
        finishFile(sharedInputFiles, currentFileNode);
        currentFileNode = getNextFileNode(sharedInputFiles, currentFileNode);
        if(!filesFinished(sharedInputFiles)){
            printf("Thread %d will now start servicing file: %d\n", tid, currentFileNode->index);
        }
    }
    ThreadInfo *values = malloc(sizeof(ThreadInfo));
    values->tid = tid;
    values->filesServiced = filesServiced;
    pthread_exit( values );
    return NULL;
}

void* resolve(void* data){
	struct Thread* thread = data;
   	char* domain;
	FILE* logFile = thread->logFile;
	pthread_mutex_t* buffmutex = thread->buffmutex;
	pthread_mutex_t* outmutex = thread->outmutex;
	ArrayBuffer* sharedBuffer = thread->sharedBuffer;
	char ipstr[MAX_IP_LENGTH];
    while(!buffer_is_empty(sharedBuffer) || !isFinished(sharedBuffer)){
		pthread_mutex_lock(buffmutex);
		domain = buffer_pop(sharedBuffer);
		if(domain == NULL){
			pthread_mutex_unlock(buffmutex);
			usleep(rand() % 100 + 5);
		}
		else {
			pthread_mutex_unlock(buffmutex);
			if(dnslookup(domain, ipstr, sizeof(ipstr)) == UTIL_FAILURE)
				strncpy(ipstr, "", sizeof(ipstr));
			printf("%s:%s\n", domain, ipstr);
            pthread_mutex_lock(outmutex);
			fprintf(logFile, "%s,%s\n", domain, ipstr);
			pthread_mutex_unlock(outmutex);
    	}
			free(domain);
	}
    return NULL;
}

int main(int argc, char * argv[]){
    clock_t start, end;
    double cpu_time_used;
    start = clock();
    if(argc<MINARGS){
        printf("ERROR: Missing arguments.\n");
		printf("Usage: %s\n", USAGE);
		return EXIT_FAILURE;
	}
    // requesters are the threads that read from the files
    int numRequesters = atoi(argv[1]);
    // resolvers are the threads that write the output to the files
    int numResolvers = atoi(argv[2]);
    char* requesterLogFileName = argv[3];
    char* resolversLogFileName = argv[4];
    int numInputFiles = argc - 5;

	ArrayBuffer sharedBuffer;
	FileBuffer sharedInputFiles;
	FILE* requestLog = fopen(requesterLogFileName, "w");;
	FILE* resolverLog = NULL;
	FILE* inputfps[numInputFiles]; //Array of inputs
	//Arrays of threads for requests and resolves
	pthread_t requests[numRequesters];
	pthread_t resolves[numResolvers];
	//Arrays of mutexes for ArrayBuffer and output file
	pthread_mutex_t buffmutex;
	pthread_mutex_t outmutex;
	struct Thread request_info[numRequesters];
	struct Thread resolve_info[numResolvers];

    printf("Requersters: %d\n", numRequesters);
    printf("Resolvers: %d\n", numResolvers);
    printf("Number input files: %d\n", numInputFiles);
    if(numRequesters > MAX_REQUESTER_THREADS){
        printf("ERROR: Maximum number of requester threads is %d\n", MAX_REQUESTER_THREADS);
		printf("Usage: %s\n", USAGE);
		return EXIT_FAILURE;
    } else if (numResolvers > MAX_RESOLVER_THREADS){
        printf("ERROR: Maximum number of resolver threads is %d\n", MAX_RESOLVER_THREADS);
		printf("Usage: %s\n", USAGE);
		return EXIT_FAILURE;
    }

	 //Open output files
    resolverLog = fopen(resolversLogFileName, "w");
	if(!requestLog){
		perror("Error Opening requester Output File");
		return EXIT_FAILURE;
	}
    if(!resolverLog){
		perror("Error Opening resolver Output File");
		return EXIT_FAILURE;
	}

    file_buffer_init(&sharedInputFiles, numInputFiles);

    for(int i=0; i<numInputFiles; i++){
		inputfps[i] = fopen(argv[i+5], "r");
        fileBufferPush(&sharedInputFiles, inputfps[i]);
	}
    // file_buffer_init(&sharedInputFiles, *inputfps, numInputFiles);

    //Initialize sharedBuffer ArrayBuffer
	buffer_init(&sharedBuffer, 15);
	//Initialize mutexes
	pthread_mutex_init(&buffmutex, NULL);
	pthread_mutex_init(&outmutex, NULL);
	//Create request pthreads
	for(int i=0; i<numRequesters; i++){
        resolve_info[i].logFile = requestLog;
		//Set data for struct to pass to current pthread
        if(i >= numInputFiles){
            request_info[i].readFile = inputfps[i - numInputFiles];
            request_info[i].currentFile = getFileNode(&sharedInputFiles, i - numInputFiles);
        }
        else{
            request_info[i].readFile = inputfps[i];
            request_info[i].currentFile = getFileNode(&sharedInputFiles, i);
        }

        request_info[i].sharedInputFiles = &sharedInputFiles;
		request_info[i].buffmutex = &buffmutex;
		resolve_info[i].outmutex = &outmutex;
		request_info[i].sharedBuffer = &sharedBuffer;
		//pthread_create(pthread_t *thread, const pthread_attr_t *attr, void *(*start_routine) (void *), void *arg); arg==id passed to request
        pthread_create(&(requests[i]), NULL, request, &(request_info[i]));
		printf("Create request %d\n", i);
    }
	// Create resolve pthreads
    for(int i=0; i<numResolvers; i++){
		resolve_info[i].logFile = resolverLog;
		resolve_info[i].buffmutex = &buffmutex;
		resolve_info[i].outmutex = &outmutex;
		resolve_info[i].sharedBuffer = &sharedBuffer;
		pthread_create(&(resolves[i]), NULL, resolve, &(resolve_info[i]));
		printf("Create resolve %d\n", i);
    }
    ThreadInfo *status;
    // Wait for request threads to complete
    for(int i=0; i<numRequesters; i++){
        // int pthread_join(pthread_t thread, void **retval); joins with a termindated thread
        pthread_join(requests[i], &status);
        printf("Thread %d serviced %d files.\n\n", status->tid, status->filesServiced);
        fprintf(requestLog, "Thread %d serviced %d files.\n", status->tid, status->filesServiced);
        free(status);
	}
	finished(&sharedBuffer);
    // Wait for resolve threads to complete
    for(int i=0; i<numResolvers; i++){
        pthread_join(resolves[i], NULL);
		printf("Finished Resolved %d \n", i);
	}
    // free ArrayBuffer
	buffer_cleanup(&sharedBuffer);
	fclose(resolverLog);
	fclose(requestLog);
    // close files and free
    closeFileBuffer(&sharedInputFiles);
	// Destroy the mutexes
	pthread_mutex_destroy(&buffmutex);
	pthread_mutex_destroy(&outmutex);
    end = clock();
    cpu_time_used = ((double) (end - start)) / CLOCKS_PER_SEC;
    printf("\nTotal runtime: %f\n\n", cpu_time_used);
    return 0;
}
