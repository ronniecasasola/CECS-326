/*
* pellet class
*
* @Ronnie Casasola
*
*/

#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <signal.h>
#include <time.h>
#include <sys/mman.h>
#include <sys/types.h>
int sharedMemoryID;
int locationIndex;

//structure of shared memory segment
struct criticalRegion{
    //postion in grid of pellet
    int manager[20];
    //number of pellets
    int pelletsNum;
};
//critical region area
struct criticalRegion *shmem;

//moves the pellets
void down(int locationIndex){
    while(shmem->manager[locationIndex] + 10 < 109){
	//checks if position of pellet and fish overlap
        if (shmem->manager[locationIndex] + 10 == shmem->manager[0]){
            break;
        }
        else{
            shmem->manager[locationIndex]+=10;
            sleep(2);
        }
    }
}

//kills processes if interrupt of keyboard input ^C
void interruptSignal(){
    printf("\nPellet died due to interruption. Pellet ID:  %d \n", getpid());
    //Detaches the memory segment of the shared memory
    shmdt(shmem);
    shmctl(sharedMemoryID, IPC_RMID, NULL);
    exit(0);  
}

//Kills process
void exitOnAlarm(){
    printf("\nPellet killed due to 30 sec timer. Pellet ID:  %d \n", getpid());
    
    //Detach shared memory
    shmdt(shmem);
    shmctl(sharedMemoryID, IPC_RMID, NULL);
	exit(0);//exit
}

int main(int argc, char  *argv[]){
	//signal for interrupts
	signal(SIGINT, interruptSignal);
	signal(SIGTERM, exitOnAlarm);

	//initializes the memory of the structure
	shmem=malloc(sizeof (struct criticalRegion));

	//initializes the key
	key_t key = 123;

	//initializes the shared memory segment
    if ((sharedMemoryID = shmget(key, sizeof(shmem), IPC_CREAT | 0666)) < 0){
        perror("shmget");
        exit(1);
	}

	//shared memory segment is attached
    	shmem=shmat(sharedMemoryID,NULL,0);

	//initialization statement
	locationIndex=-1;

	//searches the 2d array in the shared memory
	for (int i=1; i<20; i++){
	//checks if it is empty
        if(shmem->manager[i]==-1){
            locationIndex = i;
            break;
        }
}
	if(locationIndex==-1){
		//shared memory segment is detached
		shmdt(shmem);
		shmctl(sharedMemoryID, IPC_RMID, NULL);
		//exit statement
		exit(0);
	}
	//pellet count is increased
	shmem->pelletsNum += 1;
	
	//pellet is inserted in a random position
   	srand(time(NULL));
	shmem->manager[locationIndex]=rand()%10 + 10*(rand()%10);

	//moves the pellet in array
	down(locationIndex);

	//process is killed if the fish eats a pellet
	if (shmem->manager[locationIndex] + 10 == shmem->manager[0] ||
        shmem->manager[locationIndex] + 10 == shmem->manager[0] + 1 ||
        shmem->manager[locationIndex] + 10 == shmem->manager[0]-1){

	//empty in array 
        shmem->manager[locationIndex]=-1;
        printf("\nFish ate the Pellet. Pellet ID:  %d ", getpid());

	//number of pellets is decreased
        shmem->pelletsNum-=1;

	//exit statement
        exit(0);
    }
    
    //kills process if pellet is uneaten 
    if (shmem->manager[locationIndex] + 10 >= 109){

	//empty spot
        shmem->manager[locationIndex] =- 1;
        printf("\nPellet killed because it went uneaten. Pellet ID:  %d ", 		getpid());
	//number of pellets decreased
        shmem->pelletsNum -= 1;
	//exit statement
        exit(0);

    }
    
}

