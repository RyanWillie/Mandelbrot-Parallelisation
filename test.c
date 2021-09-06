#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <complex.h>
#include <time.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/socket.h>

enum {READ, WRITE};

int main(int argc, char*argv[])
{
    int numProcess = atoi(argv[1]);
	int c2p[2], p2c[2];
    char message[50];
    char result[30];
    int height = 1000;

    if (pipe(c2p) != 0) {
		perror("Pipe error"); 
	}
    if (pipe(p2c) != 0) {
		perror("Pipe error"); 
	}

	for(int i = 0; i < numProcess; i++){
		
		if(fork() == 0)
		{
            read(p2c[READ], &message, sizeof(message));
            printf("I'm child %d, I've been assigned: %s\n", i +1, message);
            strcpy(message, "Hello from your child ");
            sprintf(result, "%d", i+1);
            strcat(message, result);
            write(c2p[WRITE], message, sizeof(message));

            exit(EXIT_SUCCESS);
		}else{
			//The parent
            int chunk = height / numProcess;
            sprintf(result, "%d", i * chunk);
            strcpy(message, "Start at: ");
            strcat(message, result);
            write(p2c[WRITE], message, sizeof(message));
		}
	}

    for(int i=0;i<numProcess;i++) // loop will run n times (n=5)
    wait(NULL);
    for(int i=0; i <numProcess; i++){
        read(c2p[READ], &message, sizeof(message));
        printf("%s\n", message);
       
    }
}