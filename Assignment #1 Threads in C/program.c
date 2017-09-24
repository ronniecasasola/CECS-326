//Ronald Casasola
//012737398
//CECS 326 Lab 1
//Sep 12, 2017

// 1) In this program PTHREAD creates and runs a child thread using pthread_create. After the user inputs the enter key, the child thread is killed and then the parent thread waits for 5 seconds then prints out its message before exiting.

// 2) The sleeping thread can still output its message because it is independent from the main thread and the child thread infinitly runs with the while loop and it keeps outputting its message until the enter key is pressed.


//needed header files
#include <pthread.h>
#include <unistd.h>
#include <stdio.h>

//second (child) thread
static void *child (void *ignored)
{
    //loops infinetely
    while (1){
	//sleeps for 3 seconds then prints out message
        sleep(3);
        printf("Child is done sleeping 3 seconds. \n");    
    }
    //returns a null statement
    return NULL;   
}

//the main thread
int main( int argc, char *argv[]){
    //creates a child thread
    pthread_t child_thread;
    int code;
    
    //pthread_create creates a new thread that has a child thread
    code = pthread_create(&child_thread, NULL, child, NULL);
    
    //used for user input to stop thread
    char key_input;
    key_input = getchar();
    
    if(key_input){
        printf("User pressed the enter key \n");
    }
    //child thread will end after user input
    code = pthread_cancel(child_thread);
    

    //child will return a null 
    if (code){
        fprintf(stderr, "pthread_create failed with code %d\n", code);
        
    }
    //parent thread will run for 5 seconds to demonstrate second thread is terminated then exit
    sleep(5);
    printf("Parent is done sleeping 5 seconds. \n");
    
    return 0;
}