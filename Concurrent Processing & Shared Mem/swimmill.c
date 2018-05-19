/*
* swimmill class
*
* @Ronnie Casasola
*
*/

#include <stdio.h>
#include <sys/types.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <string.h>
#include <sys/shm.h>
#include <unistd.h>
#include <signal.h>
#include <stdbool.h> 
#include <time.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#define len 10

//structure of shared memory segment
struct criticalRegion *shmem;
//ID of shared memory
int sharedMemoryID;
//structure of shared memory
struct criticalRegion{
    //fish and pellet locations
    int manager[20];
    //counts the number of pellets in grid
    int pelletsNum;
};

//kills processes if interrupt of keyboard input ^C
void interruptSignal(){
    	printf("\nSwim mill died due to interruption. Swim mill ID:  %d ", getpid());
	//Detach memory segment of shared memory
	shmdt(shmem);
        shmctl(sharedMemoryID, IPC_RMID, NULL);
	//ends swimmill
	exit(0);
}

//converts the number position to a valid grid coordinate
void convertNumtoCoord(int pos, int index, char swimmill_array[len][len]){
    if (pos > 0){
	//calcultes the x coordinate
        int x = pos%10;
	//calculates the y coordinate
        int y  = (pos - x)/10;
        if (index == 0){
	//inserts the fish
            swimmill_array [y][x]='F';
        }
        else{
	//inserts the pellet
            swimmill_array [y][x]='P';
        }
    }
}


//displays the swimmill
void printGrid(char swimmill_array[len][len]){
    for (int i = 0; i < len; i++){
        printf("\n%d|", i);
        for (int j = 0; j < len; j++){
            printf("   %c",swimmill_array[i][j]);
        }
    }
    printf("\n   _______________________________________");
    printf("\n     1   2   3   4   5   6   7   8   9   10\n\n");
    printf("Pellet count: %d",shmem->pelletsNum);
}

//updates the swimmill
void updateGrid(char swimmill_array[len][len]){
    //updates pellets and fish
    for (int i = 0; i < 10; i++){
        for (int j = 0; j < 10; j++){
            swimmill_array[i][j] = 'x';
        }
    }
    
    //inserts fish and pellets
    for (int i=0; i<20; i++){
        convertNumtoCoord(shmem->manager[i], i, swimmill_array);
    }
    printGrid(swimmill_array);
 
}

//Kill process
void exitOnAlarm(){
    printf("\nSwim mill died after 30 seconds. Swim mill ID:  %d \n", getpid());
	
	//Detach memory segment from shared memory
	shmdt(shmem);
     shmctl(sharedMemoryID, IPC_RMID, NULL);
	//kills pellets and fish processes
	kill(0,SIGTERM);
	exit(0);
}
 
int main(int argc, char *argv[]){
	char c;
	key_t key;
	//created a 2-d array for the swim mill
	char swimmill_array[10][10];
	//sets the key
	key = 123;
	//alocates shared memory
	shmem = malloc(sizeof(struct criticalRegion));

	//created the memory segment for shared memory
	if ((sharedMemoryID = shmget(key, sizeof(shmem), IPC_CREAT | 0666)) < 0){
        perror("shmget");
	exit(1);
	}

	//attach the segment to our data space
   	shmem = shmat(sharedMemoryID, NULL, 0);

	// Put some things into the memory for the other process to read
	for(int i = 0; i < 20; i++){{
        	shmem->manager[i]=-1;
	}	

	shmem->pelletsNum = 0;
	//checks for interrupt signal
	signal(SIGINT, interruptSignal);
	//checks for alarm signal
	signal(SIGALRM, exitOnAlarm);
	//timer
	alarm(30);
	//fork statement
	pid_t fishPID = fork();

    	if(fishPID==0){
		execv("fish", argv);
	}
	
	pid_t printSwimMill = fork();
	
	if (printSwimMill==0){
		while (1){
			updateGrid(swimmill_array);
			printf("\n");
			sleep(1);
		}
	}

	while(1){
		//fork of pellets
		pid_t pellet_pid=fork();
		if(pellet_pid==0){
			execv("pellet",argv);
		}
		//limits the number of process to twenty
		while (shmem->pelletsNum>19){
			sleep(2);
		}
		//drop pellet at random times
		srand(time(NULL));
	   	sleep(rand()%10);
		}
	}

}


