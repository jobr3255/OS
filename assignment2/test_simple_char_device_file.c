#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include<errno.h>

#define DEVICE "/dev/simple_character_device"
#define BUFFER_SIZE 1024
#define BUFFER_LENGTH 1024



void displayMenu(){
    printf("\
Press r to read from device\n\
Press w to write to the device\n\
Press s to seek into the device\n\
Press e to exit from the device\n\
Press anything else to keep reading or writing from the device\n\
Enter command: ");
}

int main(){
    char stringToSend[BUFFER_SIZE];
    char userInput, temp;

    int *readInput = 0, *seekOffset = 0, *seekWhence = 0;
    char buff[BUFFER_SIZE];
    static char receive[BUFFER_SIZE];
    int file = open(DEVICE , O_RDWR);

    while(userInput != 'e'){
        displayMenu();
        scanf("%c", &userInput);
        tolower(userInput);
        switch(userInput){
            case 'r':

                printf("Enter the number of bytes you want to read: ");
                scanf("%d", &readInput);
                char *readBuffer;
                readBuffer = malloc((size_t)(readInput));
                //printf("%c\n", &readBuffer);
                if(readBuffer == NULL){
                    printf("Error! memory not allocated.!\n");
                    exit(0);
                }

                // should return a positive number (# of bytes being read)
                int ret = read(file, readBuffer, readInput);

                if (ret < 0){
                   perror("Failed to read the message from the device.");
                   return errno;
                }

                printf("The received message is: [%s]\n", readBuffer);
                free(readBuffer);

                while(getchar() != '\n');
                break;

            case 'w':

                printf("Type in a short string to send to the kernel module: ");
                // temp statement to clear buffer
                scanf("%c",&temp);
                // %[^\n] allows for spaces to be entered
                scanf("%[^\n]", stringToSend);

                printf("Writing message to the device [%s].\n", stringToSend);
                ret = write(file, stringToSend, strlen(stringToSend));
                if (ret < 0){
                   perror("Failed to write the message to the device.");
                   return errno;
                }

                while(getchar() != '\n');
                break;

            case 's':
                printf("Enter an offset value: \n");
                scanf("%d", &seekOffset);
                printf("Enter a value for whence(third parameter): \n");
                scanf("%d", &seekWhence);
                llseek(file, seekOffset, seekWhence);

                while(getchar() != '\n');
                break;
        }
    }

    close(file);
    printf("Exiting\n");
    return 0;
}
