#include <signal.h>
#include <stdlib.h>
#include <sys/sem.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <stdio.h>
//define the size of the shared memory
#define shmem     20*sizeof(int)
//initialize variables
int shmid;
//key for semget
key_t semkey; 
//nsems to pass to segment
int nsems; 
//return value from the semget
int semid; 

//returns the value of the semaphore
int getVal()
{
	//initialize the variable
	int val;
	//semctl is for semaphore control operation
	//if abnormal termination
	if ( semctl(semid,0,GETVAL) == -1 )
    	{
     		perror("semctl failure reason:");
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
      		perror("Lock failure Reason:");
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
      		perror("Unlock failure Reason:");
		return 0;
    	} else { return 1; }
}

//runs when process is interupted
oninterrupt(void) 
{
	//prints the pid
	fprintf(stderr, "Killed pellet, pid: %d\n", getpid()); 
	exit(0);
}

//runs the pellet process
main()
{
	//set up the interrupt signal
	signal(SIGINT, oninterrupt); 
	//set up the pid
	srand(getpid()); 
	key_t key;
	//initialize
	int *shm, *swimmill;
	//use the same key as the swimmill process
	key = 50; 

	//find the memeory segment in shared memory
	if ((shmid = shmget(key, shmem, 0666)) < 0) {
		perror("shmget");
		exit(1);
	}

	//attach the memory segment the shm pointer
	if ((shm = shmat(shmid, NULL, 0)) == (int *) -1) {
		perror("shmat");
		exit(1);
	}

	//set the swimmill pointer to the shm pointer
	swimmill = shm;
	//set semkey
	semkey = 51; 
	//only using one semaphore
	nsems = 1; 

	//set up the semaphore
	if ((semid = semget(semkey, nsems, IPC_CREAT | 0666)) == -1) {
		perror("semget: semget failed"); 
		exit(1);
	}
	//create a random int for the pellet position in swimmill
	int position = rand() % 100; 
	//represents the location in shared memory
	int number = 0; 

	//picks the shared memory location
	while(getVal() < 1)
	{
		//printf("locked");
	};
	//locks the semaphore
	lock();
	for(int i = 1; i < 20; i++) {
		//sets to an unused slot
		if(*(swimmill + i) == 103) {
			//sets the number to that location 
			number = i; 
			//100 represents taken slot
			*(swimmill + i) = 100; 
			//exit statement for for loop
			i = 20; 
		}
	}
	//unlocks the semaphore
	unlock();

	//first 3 pellets automatically run
	//wait for the pellet 3 spots behind to run if not in the first 3 		//helps regulate pellet drop

	if(number > 3) {	
		//initialize variable
		int previous = 0;
		//represents the finished state
		while(previous != 101) { 
			//sleeps for 1 second
			sleep(1);
			//while less than 1
			while(getVal() < 1){
			//printf("locked");
			};
			//locks the semaphore
			lock();
			previous = *(swimmill + number - 3);
			//unlocks the semaphore
			unlock();
		}
	}
	while(getVal() < 1)
	{
		//printf("locked");
	};
	//locks the semaphore
	lock();
	//sets the position into shared memory
	*(swimmill + number) = position; 
	//unlocks the semaphore
	unlock();

	//while pellet is still on map, or at -100 which represents eaten 
	//pellet, update pellet position and check for fish
	while(position >= 0 || position == -100) {
		//sleeps for 1 second
		sleep(1);
		//while less than 1
		while(getVal() < 1){
			//printf("locked");
		};
		//locks the semaphore
		lock();
		//checks if pellet has been eaten
		if(*(swimmill + number) == -100) 
		{
			//print that it was eaten 
			printf("Pellet was eaten. pid %d, position %d\n", getpid(), position);
	
			fprintf("Pellet was eaten pid %d, position %d\n", getpid(), position);
	
			//update the position to represent an eaten pellet
			position = -100; 
		}	
		//moves pellet downstream	
		position -= 10; 
		//updates the position in shared memory
		*(swimmill + number) = position; 
		//unlocks the semaphore
		unlock();
	}
	//runs if pellet moved off stream without being eaten
	if(position > -100) 
	{
		//print that fish missed it
		printf("Pellet was not eaten pid %d, position %d\n", getpid(), (position + 10));
		
		fprintf( "Pellet was not eaten pid %d, position %d\n", getpid(), (position + 10));
	
	}
	while(getVal() < 1)
	{

		//printf("locked");
	};
	//locks the semaphore
	lock();
	//update shared memory to represent a finished pellet
	*(swimmill + number) = 101; 
	//unlocks the semaphore
	unlock();	
	//clean
	oninterrupt();
	//exit
    exit(0);
}
