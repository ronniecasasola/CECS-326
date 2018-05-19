#include <signal.h>
#include <stdlib.h>
#include <sys/sem.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <stdio.h>
#include <unistd.h>
//define size of shared memory
#define shmem     20*sizeof(int) 
//initialize variables
int shmid;
int *shm;
//key for semget
key_t semkey; 
//nsems to pass to segment
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
	if ( semctl(semid,0,GETVAL) == -1 )
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
      		perror("lock failure Reason:");
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

//sets the semaphore to a certain value
int setTo(int n)
{
	if ( semctl(semid,0,SETVAL,n)<0 )
    	{
     		perror("semctl failure reason:");
      		exit(-1);
        }
}

//used when the process is killed
void onkill()
{
	printf("Exiting\n");
	deletemem();
	semctl(semid, 0, IPC_RMID, 0);
	sleep(1);
	exit(0);
}

//frees the shared memory segment
void deletemem()
{
	shmdt(shm); //delete the pointer
	shmctl(shmid, IPC_RMID, NULL);
}
//runs the swimmill 
main()
{
	// a signal to kill the process
	signal(SIGINT, onkill); 
	int *swimmill;

	//create a key for shared memory
	int key = 50; 

	//create the shared memory segment
	if ((shmid = shmget(key, shmem, IPC_CREAT | 0666)) < 0) { 
		perror("shmget");
        	exit(1);
    	}
    
	//attach memory segment to shm pointer
	if ((shm = shmat(shmid, NULL, 0)) == (char *) -1) {
   	     perror("shmat");
   	     exit(1);
    	}
	//set semkey
	semkey = 51; 
	//only using one semaphore
	nsems = 1; 

	//set up the semaphore
	if ((semid = semget(semkey, nsems, IPC_CREAT | 0666)) == -1) {
		perror("semget: semget failed"); 
		exit(1);
	}
	//initialize the segment to unlocked
	setTo(1); 
	//attach shm to the swimmill pointer
    	swimmill = shm; 
    	for(int i = 1; i < 20; i++) {
		while(getVal() < 1){
			//printf("locked");
		};
		//locks the semaphore
		lock();
		//set the pellet shared memory for uncreated pellet
		*(swimmill + i) = 103; 
		//unlocks the semaphore
		unlock();
    	}
	//int array to store pids
	int pids[20]; 
	//create the pellet processes
	for(int j = 0; j < 19; j++) {
     		pid_t pid=fork();
     		if (pid==0) {
			//child processes
        		execv("pellet",NULL);
			//exit if it fails
        		exit(127); 
    		}
		//else add pid to pid array
    		else { 
			*(pids + 1 + j) = pid; 
    		}
  	}

	//create a fish process
	pid_t pid=fork(); 
     	if (pid==0) {
        	execv("fish",NULL);
		//exit if it fails
        	exit(127); 
    	}
	//else add fish pid to the array
    	else { 
		*(pids) = pid; 
    	}
//initialize counter for execution time
   int counter = 0; 
//while counter less than thirty
   while (counter < 30) {
 //runs for 30 seconds
	//sleeps for 1 second
	sleep(1); 
	//set up the default board	
	char board[100] =  "----------------------------------------------------------------------------------------------------";
	//initializes the pellets in the swimmill
	while(getVal() < 1)
	{
		//printf("locked");
	};
	//locks semaphore
	lock();
	for(int i = 1; i < 20; i++) {
		if(*(swimmill+i) < 100) {
			*(board + *(swimmill+i)) = 'p';
		}
	}
	//initializes the fish in the stream
	*(board + *(swimmill)) = 'f';
	//unlocks semaphore
	unlock();

	//prints the board
	for(int i = 99; i >= 0; i--) {
		putchar(*(board+i));
		if((i)%10 == 0) printf("\n");		
	}
	printf("\n");
	//increment the time counter
	counter++; 
    }

	//kills all the child processes once the time counter runs out
	for(int i = 0; i < 20; i++)
	{
		kill(*(pids + i), SIGINT);
	}
	//performs clean up
	onkill(); 
	//exit statement
	exit(0);
}
