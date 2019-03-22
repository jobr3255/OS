#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <pthread.h>
#include <unistd.h>
#include <stdbool.h>
#include <sys/types.h>
#include "Queue.c"
#include "util.c"

#define USAGE "multi-lookup <# requester> <# resolver> <requester log> <resolver log> [ <data file> ...]"
#define MINARGS 6
#define ERROR "\033[91mERROR\033[0m"

#define MAX_INPUT_FILES 10
#define MAX_RESOLVE_THREADS 10
#define MAX_REQUEST_THREADS 5
#define MAX_IP_LENGTH INET6_ADDRSTRLEN
#define SBUFSIZE 1025
#define INPUTFS "%1024s"

struct Thread{
	Queue* buffer;
    FILE* read_file;
    FILE* log_file;
    pthread_mutex_t* request_mutex;
    pthread_mutex_t* resolve_mutex;
};

void* request(void* id){
	struct Thread* thread = id; //Make a thread to hold info
   	char hostname[MAX_NAME_LENGTHS]; //hostname char arrays
	char *domain;
	bool done = false; //flag
	FILE* inputfp = thread->read_file; //Input file
	FILE* log_file = thread->log_file; //Input file
	pthread_mutex_t* request_mutex = thread->request_mutex; //Buffer mutex
	Queue* buffer = thread->buffer; //Queue
   	while(fscanf(inputfp, INPUTFS, hostname) > 0){ //Read input and push onto buffer
		while(!done){ //Repeat until the current hostname is pushed to the queue
			domain = malloc(SBUFSIZE); //allocate memory for dmain
			strncpy(domain, hostname, SBUFSIZE); //copy hostname to domain, max number of characters
			pthread_mutex_lock(request_mutex);//Lock the buffer, or try
			while(is_full(buffer)){ //When queue is full
					pthread_mutex_unlock(request_mutex); 	//Unlock the buffer, or try
					usleep(rand() % 100 + 5); //Wait 0-100 microseconds
					pthread_mutex_lock(request_mutex); //Lock the buffer, or try
			}
        	push(buffer, domain);
			pthread_mutex_unlock(request_mutex); //Unlock!
			done = true; //Indicate that this hostname was pushed successfully
    	}
		done = false; //Reset pushed for the next hostname
	}
    printf("Thread %d serviced %d files.", getpid(), 1);
    return NULL;
}

void* resolve(void* id){
	struct Thread* thread = id; //Make a thread to hold info
   	char* domain; //domain char arrays
	FILE* outfile = thread->log_file; //Output file
	pthread_mutex_t* request_mutex = thread->request_mutex; //Buffer mutex
	pthread_mutex_t* resolve_mutex = thread->resolve_mutex; //Output mutex
	Queue* buffer = thread->buffer; //Queue
	char ipstr[MAX_IP_LENGTH]; //IP Addresses
    while(!is_empty(buffer)){ //while the queue has stuff or there's request threads, loop
		pthread_mutex_lock(request_mutex); //lock buffer
		domain = pop(buffer); //pop off queue
		if(domain == NULL){ //if empty, unlock
			pthread_mutex_unlock(request_mutex);
			usleep(rand() % 100 + 5);
		}
		else { //Unlock and go!
			pthread_mutex_unlock(request_mutex);
			if(dnslookup(domain, ipstr, sizeof(ipstr)) == UTIL_FAILURE)//look up domain, or try
				strncpy(ipstr, "", sizeof(ipstr));
			printf("%s:%s\n", domain, ipstr);
            pthread_mutex_lock(resolve_mutex); //lock output file, if possible
			fprintf(outfile, "%s,%s\n", domain, ipstr); //write to output file
			pthread_mutex_unlock(resolve_mutex); //unlock output, if possible
    	}
			free(domain);
	}
    return NULL;
}

int main(int argc, char* argv[])
{
    if(argc<MINARGS){
        printf("%s: Missing arguments.\n", ERROR);
		printf("Usage: %s\n", USAGE);
		return EXIT_FAILURE;
	}
    // requesters are the threads that read from the files
    unsigned int numRequesters = atoi(argv[1]);
    // resolvers are the threads that write the output to the files
    unsigned numResolvers = atoi(argv[2]);
    char* requesterLogFileName = argv[3];
    char* resolversLogFileName = argv[4];
    int numInputFiles = argc - 5;

    printf("Requersters: %d\n", numRequesters);
    printf("Resolvers: %d\n", numResolvers);
    printf("Number input files: %d\n", numInputFiles);
    if(numRequesters > MAX_REQUEST_THREADS){
        printf("%s: Maximum number of requester threads is %d\n", ERROR, MAX_REQUEST_THREADS);
		printf("Usage: %s\n", USAGE);
		return EXIT_FAILURE;
    }else if(numResolvers > MAX_RESOLVE_THREADS){
        printf("%s: Maximum number of resolver threads is %d\n", ERROR, MAX_RESOLVE_THREADS);
		printf("Usage: %s\n", USAGE);
		return EXIT_FAILURE;
    }
    // Array of input files
    FILE* inputFiles[numInputFiles];

    // arrays of requester and resolver threads
    pthread_t requesterThreads[numRequesters];
	pthread_t resolverThreads[numResolvers];
    // mutex locks
    pthread_mutex_t requestMutex;
	pthread_mutex_t resolveMutex;
    //Initialize mutexes
	pthread_mutex_init(&requestMutex, NULL);
	pthread_mutex_init(&resolveMutex, NULL);

    // holds Thread data objects
	struct Thread request_data[numRequesters];
	struct Thread resolve_data[numResolvers];

    struct Queue sharedBuffer;
    init(&sharedBuffer, 5);
    FILE* requesterLog = fopen(requesterLogFileName, "w");
    FILE* resolverLog = fopen(resolversLogFileName, "w");

    int i;
    for(i=0; i<numInputFiles; i++){ 	//Open input files or try at least
		inputFiles[i] = fopen(argv[5 + i], "r");
        if(inputFiles[i] == NULL)
            printf("file %s failed to open\n", argv[6 + i]);
	}

    // create requester threads
    for(i=0; i<numRequesters; i++){
		//Set data for struct to pass to current pthread
		request_data[i].read_file = inputFiles[i];
		request_data[i].log_file = requesterLog;
		request_data[i].request_mutex = &requestMutex;
		request_data[i].resolve_mutex = NULL;
		request_data[i].buffer = &sharedBuffer;
		//pthread_create(pthread_t *thread, const pthread_attr_t *attr, void *(*start_routine) (void *), void *arg); arg==id passed to request
        pthread_create(&(requesterThreads[i]), NULL, request, &(request_data[i]));
		printf("Create request %d \n", i);
    }
    // create resolver threads
    for(i=0; i<numResolvers; i++){
		resolve_data[i].log_file = resolverLog;
		resolve_data[i].request_mutex = &requestMutex;
		resolve_data[i].resolve_mutex = &resolveMutex;
		resolve_data[i].buffer = &sharedBuffer;
		pthread_create(&(resolverThreads[i]), NULL, resolve, &(resolve_data[i]));
		printf("Create resolve %d \n", i);
    }
    // Wait for request threads
    for(i=0; i<numRequesters; i++){
        pthread_join(requesterThreads[i], NULL);
		printf("Requested %d \n", i);
	}

    for(i=0; i<numResolvers; i++){
        pthread_join(resolverThreads[i], NULL); // int pthread_join(pthread_t thread, void **retval); joins with a termindated thread
		printf("Resolved %d \n", i);
	}

    cleanup(&sharedBuffer);

    fclose(requesterLog);
    fclose(resolverLog);
    // Close input files
    for(i=0; i<numInputFiles; i++){
        if(inputFiles[i] != NULL)
		    fclose(inputFiles[i]);
        else
            printf("file %d is not open\n", i);
	}
	// Destroy the mutexes
	pthread_mutex_destroy(&requestMutex);
	pthread_mutex_destroy(&resolveMutex);
    printf("done\n");
    return 0;
}
