/*
* fish class
*
* @Ronnie Casasola
*
*/

#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/shm.h>
#include <unistd.h>
#include <signal.h> 
#include <time.h>
#include <sys/mman.h>
#include <sys/types.h>

//structure of shared memory segment
struct criticalRegion{
//has position of fish in 2d array 
    int manager[20];
//number of pellets in shared memory
    int pelletsNum;
};

struct criticalRegion *shmem;
//id of shared memory
int sharedMemoryID;
int locationIndex; 

//kills processes if interrupt of keyboard input ^C
void interruptSignal(){
    printf("\nFish died due to interruption. Fish ID:  %d \n", getpid());
    
   //Detaches the memory segment of the shared memory
    shmdt(shmem);
    shmctl(sharedMemoryID, IPC_RMID, NULL);
    exit(0);//exit
    
}

//kills process
void exitOnAlarm(){
    printf("\n30 sec timer finished, fish killed. Fish ID:  %d \n", getpid());
    
    //Detaches the memory segment of the shared memory
    shmdt(shmem);
    shmctl(sharedMemoryID, IPC_RMID, NULL);
	kill(0, SIGTERM);
	//exit statement
	exit(0);
    
}

//searches the 2d array to eat a pellet
void moveTowardNearestPellet(){

	//position of the fish
    int fishLoc = shmem->manager[0];
    //closest possible
    int max =0; 

    //nearest position from the edge
    int nearestInColumn=0;
    int nearestInColumnDist=11;
   
   //ierates throught the shared memory
    for (int i=1; i<20; i++){

	//searches within the 2d array of shared memory
        if(shmem->manager[i]>=0 &&shmem->manager[i] < 100){
            if (shmem->manager[i]/10>=max)
            {
		//the nearest row in shared memory
                max =shmem->manager[i];
            }
            
            if(abs(shmem->manager[i]%10-fishLoc%10) <= nearestInColumnDist)
            {
                nearestInColumn=shmem->manager[i];
	//closest column distance in shared memory array
                nearestInColumnDist = abs(shmem->manager[i]%10-fishLoc%10);
            }
        }
}

printf("\nThe value of the nearest row: %d", max);
    printf("\nValue of the nearest column: %d",nearestInColumn);
	//distance of vertical position
    int nearestRowDist = max/10-9;

	// checks if row less than column distance
    if(abs(nearestRowDist)< nearestInColumnDist){
        if (max%10-fishLoc>0 && fishLoc!=99) {
	//moves to the right
            shmem->manager[0]=shmem->manager[0]+1;
            
        }
        else if (max%10-fishLoc<0 && fishLoc!=90){
	//moves to the left
            shmem->manager[0]=shmem->manager[0]-1;
        }
    }
    else{
 	//if pellet is right
        if(nearestInColumn%10-fishLoc%10 > 0 && fishLoc != 99){
	//moves the pellet in shared memory
            shmem->manager[0]=shmem->manager[0]+1;
        }
	//if pellet is on the left
        else if (nearestInColumn%10-fishLoc%10 > 0 && fishLoc != 90){
	//moves the pellet
            shmem->manager[0]=shmem->manager[0]- 1;
        }
    }
}


int main(int argc, char *argv[]){
	//signal for interrupts
	signal(SIGINT, interruptSignal);
	signal(SIGTERM, exitOnAlarm);

	//initializes the memory of the structure
	shmem=malloc(sizeof (struct criticalRegion));

	//initializes the key
	key_t key = 123;

	//initializes the shared memory segment
	 if ((sharedMemoryID = shmget(key, sizeof(shmem), IPC_CREAT | 0666)) <0){
        perror("shmget");
        exit(1);
	}

	//Attaches the memory segment in shared memory
	shmem=shmat(sharedMemoryID,NULL,0);
	
	//location in shared memory array
	srand(time(NULL));
	//location is the middle of the last row
	shmem->manager[0]=rand()%10 + 90;

	while (1){
	//searches for pellet and moves the fish in swimmill
        moveTowardNearestPellet();
        sleep(1);
	}
}

	
