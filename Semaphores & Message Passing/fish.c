#include <stdio.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <signal.h>
#include <stdlib.h>
#include <sys/sem.h>
//define size of the shared memory
#define shmem     20*sizeof(int) 
//key for semget()
key_t semkey;
//in order to pass to segment
int nsems; 
//return value from the semget
int semid; 

//returns the value of the semaphore
int getVal()
{
	//initialize variable
	int val;
	//semctl is for semaphore control operation
	//if abnormal termination
	if (semctl(semid,0,GETVAL) == -1 )
    	{
     		perror("semctl failure Reason:");
      		exit(-1);
        } else {
		return semctl(semid,0,GETVAL);
	}
	 
}

//used to lock the semaphore
int lock()
{
	struct sembuf sem_lock = { 0, -1, IPC_NOWAIT};
	//semop is for user-defined array of semaphore operations
	//if lock fails
	if(semop(semid,&sem_lock,1) == -1)
    	{
      		perror("lock failure reason:");
		return 0;

    	} else { return 1; }
}

//used to unlock the semaphore
int unlock()
{
	struct sembuf sem_unlock = { 0, 1, IPC_NOWAIT};
	//semop is for user-defined array of semaphore operations
	//if lock fails
	if(semop(semid,&sem_unlock,1) == -1)
    	{
      		perror("unlock failure reason:");
		return 0;

    	} else { return 1; }
}

//used when the process is killed
void onkill()
{
	//display that fish process has been killed
	printf("Fish was killed\n"); 
	exit(0);
}

//runs the fish process
main()
{
	//a signal to catch killed fish
	signal(SIGINT, onkill); 
	//random generation based on pid
	srand(getpid()); 
	//initialize variables
	int shmid;
	key_t key;
	int *shm, *swimmill;

	//use same key as swimmill
	key = 50; 

	//locate the shared memory segment
	if ((shmid = shmget(key, shmem, 0666)) < 0) {
		perror("shmget");
		exit(1);
	}

    	//attach shared memory segment to shm pointer
    	if ((shm = shmat(shmid, NULL, 0)) == (int *) -1) {
        	perror("shmat");
        	exit(1);
    	}
	//set semkey
	semkey = 51;
	//only using one semaphore 
	nsems = 1; 

	//set up the semaphore
	if ((semid = semget(semkey, nsems, IPC_CREAT | 0666)) == -1) {
		perror("semget: semget has failed"); 
		exit(1);
	}

    	//set swimmill pointer to shm
    	swimmill = shm;
	//initialize the fish in middle of swimmill stream
	int position = 4; 

	while(getVal() < 1){
		//printf("locked");
	};
	//lock semaphore
	lock();
	//set position in shared memory
	*(swimmill) = position; 
	//unlock semaphore
	unlock();

	//keeps running
	while(1) {
		//time regulation
		sleep(1); 
		//stores the position of the closest pellet 
		int closest = 1; 
		//stores the  shortest distance from a pellet
		int bestDis = 1000; 
		//stores the x position of the closest pellet
		int bestX = 4;
		//pellet position, x, y, & distance from fish 
		int pos, x, y, dis; 

		while(getVal() < 1)
		{

		//printf("locked");
		};
		//lock semaphore
		lock();
		//iterate and check each pellet
		for(int i = 1; i < 20; i++) { 
			//get pellet position
			pos = *(swimmill + i); 
			if(pos == position) {
			//represents that pellet has been eaten
				*(swimmill + i) = -100; 
			} //else if pellet hasn't been eaten
				else if(pos < 100) {
				//calculate x		
				x = pos % 10; 
				//calculate y
				y = (pos - (pos % 10)) / 10; 
				//calculate the distance
				dis = (pos - x)*(pos - x) + y * y; 
				if(dis < bestDis) { 
					//finds the shortest distance
					bestDis = dis;
					closest = i;
					bestX = x;
				}
			} else {
				//do nothing
			}
		}
		//unlocks semaphore
		unlock();
		//move left if closest pellet is to the left
		if(bestX > position) position++; 
		//move right if closest pellet is to the right
		if(bestX < position) position--; 
		while(getVal() < 1)
		{
			//printf("locked");
		};
		//locks semaphore
		lock();
		//update the fish's position in shared memory
		*(swimmill) = position; 
		//unlocks semaphore
		unlock();
	}

    exit(0);
}
