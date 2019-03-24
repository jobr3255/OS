
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <pthread.h>
#include <unistd.h>
#include <stdbool.h>
#include "util.c"
#include "ArrayBuffer.c"

#define USAGE "multi-lookup <# requester> <# resolver> <requester log> <resolver log> [ <data file> ...]"
#define MINARGS 6

//Froom lookup.c
#define SBUFSIZE 1025
#define INPUTFS "%1024s"
#define MAX_INPUT_FILES 10
#define MAX_RESOLVER_THREADS 10
#define MAX_REQUESTER_THREADS 10
#define MAX_NAME_LENGTHS 1025
#define MAX_IP_LENGTH INET6_ADDRSTRLEN
bool requests_exist = true;

//Threads
struct thread{
	ArrayBuffer* sharedBuffer;
    FILE* thread_file;
    pthread_mutex_t* buffmutex;
    pthread_mutex_t* outmutex;
};

void* request(void* id){
	struct thread* thread = id; //Make a thread to hold info
   	char hostname[MAX_NAME_LENGTHS]; //hostname char arrays
	char *domain;
	bool done = false; //flag
	FILE* inputfp = thread->thread_file; //Input file
	pthread_mutex_t* buffmutex = thread->buffmutex; //Buffer mutex
	ArrayBuffer* sharedBuffer = thread->sharedBuffer; //Queue

   	while(fscanf(inputfp, INPUTFS, hostname) > 0){ //Read input and push onto sharedBuffer
		while(!done){ //Repeat until the current hostname is pushed to the ArrayBuffer
			domain = malloc(SBUFSIZE); //allocate memory for dmain
			strncpy(domain, hostname, SBUFSIZE); //copy hostname to domain, max number of characters
			pthread_mutex_lock(buffmutex);//Lock the sharedBuffer, or try
			while(queue_is_full(sharedBuffer)){ //When ArrayBuffer is full
					pthread_mutex_unlock(buffmutex); 	//Unlock the sharedBuffer, or try
					usleep(rand() % 100 + 5); //Wait 0-100 microseconds
					pthread_mutex_lock(buffmutex); //Lock the sharedBuffer, or try
			}
        	queue_push(sharedBuffer, domain);
			pthread_mutex_unlock(buffmutex); //Unlock!
			done = true; //Indicate that this hostname was pushed successfully
    	}
		done = false; //Reset pushed for the next hostname
	}
    return NULL;
}

void* resolve(void* id){
	struct thread* thread = id; //Make a thread to hold info
   	char* domain; //domain char arrays
	FILE* outfile = thread->thread_file; //Output file
	pthread_mutex_t* buffmutex = thread->buffmutex; //Buffer mutex
	pthread_mutex_t* outmutex = thread->outmutex; //Output mutex
	ArrayBuffer* sharedBuffer = thread->sharedBuffer; //Queue
	char ipstr[MAX_IP_LENGTH]; //IP Addresses
    while(!queue_is_empty(sharedBuffer) || requests_exist){ //while the ArrayBuffer has stuff or there's request threads, loop
		pthread_mutex_lock(buffmutex); //lock sharedBuffer
		domain = queue_pop(sharedBuffer); //pop off ArrayBuffer
		if(domain == NULL){ //if empty, unlock
			pthread_mutex_unlock(buffmutex);
			usleep(rand() % 100 + 5);
		}
		else { //Unlock and go!
			pthread_mutex_unlock(buffmutex);
			if(dnslookup(domain, ipstr, sizeof(ipstr)) == UTIL_FAILURE)//look up domain, or try
				strncpy(ipstr, "", sizeof(ipstr));
			printf("%s:%s\n", domain, ipstr);
            pthread_mutex_lock(outmutex); //lock output file, if possible
			fprintf(outfile, "%s,%s\n", domain, ipstr); //write to output file
			pthread_mutex_unlock(outmutex); //unlock output, if possible
    	}
			free(domain);
	}
    return NULL;
}

int main(int argc, char * argv[]){

    if(argc<MINARGS){
        printf("ERROR: Missing arguments.\n");
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


	//Local variables (lookup.c)
	ArrayBuffer sharedBuffer; //Hostname ArrayBuffer
	FILE* outfile   = NULL; //output pointer
	FILE* inputfps[numInputFiles]; //Array of inputs
	//Arrays of threads for requests and resolves
	pthread_t requests[numRequesters];
	pthread_t resolves[numResolvers];
	//Arrays of mutexes for ArrayBuffer and output file
	pthread_mutex_t buffmutex;
	pthread_mutex_t outmutex;
	struct thread request_info[numRequesters];
	struct thread resolve_info[numResolvers];

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

	outfile = fopen(resolversLogFileName, "w"); //Open output fileor try at least
	if(!outfile){
		perror("Error Opening Output File");
		return EXIT_FAILURE;
	}
	int i;
	for(i=0; i<numInputFiles; i++){ 	//Open input files or try at least
		inputfps[i] = fopen(argv[i+5], "r");
	}
	queue_init(&sharedBuffer, 5); //Initialize sharedBuffer ArrayBuffer
	//Initialize mutexes
	pthread_mutex_init(&buffmutex, NULL);
	pthread_mutex_init(&outmutex, NULL);
	//Create request pthreads
	for(i=0; i<numRequesters; i++){
		//Set data for struct to pass to current pthread
        if(i >= numInputFiles)
            request_info[i].thread_file = inputfps[i - numInputFiles];
        else
            request_info[i].thread_file = inputfps[i];

        //request_info[i].thread_file = inputfps[i];

		request_info[i].buffmutex   = &buffmutex;
		request_info[i].outmutex    = NULL;
		request_info[i].sharedBuffer      = &sharedBuffer;
		//pthread_create(pthread_t *thread, const pthread_attr_t *attr, void *(*start_routine) (void *), void *arg); arg==id passed to request
        pthread_create(&(requests[i]), NULL, request, &(request_info[i]));
		printf("Create request %d\n", i);
    }
	//Create resolve pthreads
    for(i=0; i<numResolvers; i++){
		resolve_info[i].thread_file = outfile;
		resolve_info[i].buffmutex   = &buffmutex;
		resolve_info[i].outmutex    = &outmutex;
		resolve_info[i].sharedBuffer      = &sharedBuffer;
		pthread_create(&(resolves[i]), NULL, resolve, &(resolve_info[i]));
		printf("Create resolve %d\n", i);
    }
    //Wait for request threads to complete
    for(i=0; i<numRequesters; i++){
        pthread_join(requests[i], NULL);
		printf("Requested %d \n", i);
	}
	requests_exist=false;
    //Wait for resolve threads to complete
    for(i=0; i<numResolvers; i++){
        pthread_join(resolves[i], NULL); // int pthread_join(pthread_t thread, void **retval); joins with a termindated thread
		printf("Resolved %d \n", i);
	}
	queue_cleanup(&sharedBuffer); //Clean ArrayBuffer
	fclose(outfile);	//Close output file
	for(i=0; i<numInputFiles; i++){
		fclose(inputfps[i]); //Close input files
	}
	//Destroy the mutexes
	pthread_mutex_destroy(&buffmutex);
	pthread_mutex_destroy(&outmutex);
    return 0;
}
