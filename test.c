#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <complex.h>
#include <time.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/socket.h>

int main(int argc, char*argv[])
{
    int numProcess = atoi(argv[1]);
	int sv[2];
    char message[50];
    char result[30];
    int height = 1000;
    if(socketpair(AF_UNIX, SOCK_DGRAM, 0, sv) != 0){
        perror("Socket error\n");
    }else{
        printf("Created Socket %d\n");
    } 

	for(int i = 0; i < numProcess; i++){
		
		if(fork() == 0)
		{
			if(getpid() == getppid() + i + 1){
                read(sv[0], message, sizeof(message));
                printf("I'm child %d, I've been assigned: %s\n", i +1, message);
				strcpy(message, "Hello from your child ");
                sprintf(result, "%d", i+1);
                strcat(message, result);
				write(sv[0], message, sizeof(message));
			}else{
                printf("No idea who i am %d and parent %d\n", getpid(), getppid());
            }
            exit(EXIT_SUCCESS);
		}else{
			//The parent
            int chunk = height / numProcess;
            sprintf(result, "%d", i * chunk);
            strcpy(message, "Start at: ");
            strcat(message, result);
            write(sv[1], message, sizeof(message));
		}
	}
    for(int i=0;i<numProcess;i++) // loop will run n times (n=5)
    wait(NULL);
    for(int i=0; i <numProcess; i++){
		read(sv[1], message, sizeof(message));
        printf("%s\n", message);
       
    }
}