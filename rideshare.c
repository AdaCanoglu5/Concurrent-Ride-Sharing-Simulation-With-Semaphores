#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <stdbool.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <pthread.h>
#include <semaphore.h>

pthread_mutex_t lock;
sem_t ASemaphore;
sem_t BSemaphore;
sem_t driverSemaphore;
sem_t inProgressSem;

int ACount = 0;
int BCount = 0;

int AFan = 0;
int BFan = 0;

int cars = 0;
int carFound = 0;

bool inProgress = 0;
int queue = 0;

void *threadFunction(void *arg) {
	
	pthread_mutex_lock(&lock);
	while(inProgress == 1) {
		pthread_mutex_unlock(&lock);
		queue++;
		sem_wait(&inProgressSem);
		pthread_mutex_lock(&lock);
	}

	pthread_t tid = pthread_self();
        char Team = *((int *)arg);
        printf("Thread ID: %ld, Team: %c, I am looking for a car\n", tid, Team);
        
        bool driver = 0;
        
        if(Team == 'A')
		AFan++;
	else if(Team == 'B')
		BFan++;
	
	if (AFan == 4 || BFan == 4) {
		
            	if(Team == 'A'){
            		for(int i = 0; i < 3; i++) {
            			sem_post(&ASemaphore);
            		}
            		AFan = 0;
    		}
            	else if(Team == 'B') {
            		for(int i = 0; i < 3; i++) {
    				sem_post(&BSemaphore);
    			}
    			BFan = 0;
    		}
            	
            	driver = 1;
            	inProgress = 1;
            	pthread_mutex_unlock(&lock);
	} 
	else if (BFan >= 2 && AFan >= 2) {
          	
		if(Team == 'A')
			sem_post(&BSemaphore);
		else if(Team == 'B')
			sem_post(&ASemaphore);	
            			
            	sem_post(&ASemaphore);
            	sem_post(&BSemaphore);
            	
		AFan -= 2;
		BFan -= 2;
		
		driver = 1;
		inProgress = 1;
		pthread_mutex_unlock(&lock);
	} 
	else {	
		pthread_mutex_unlock(&lock);
		
		if(Team == 'A')
            		sem_wait(&ASemaphore);
            	else if(Team == 'B')
            		sem_wait(&BSemaphore);
	}
	
	pthread_mutex_lock(&lock);
	printf("Thread ID: %ld, Team: %c, I have found a spot in a car \n", tid, Team);
	carFound++;
	if(carFound == 4) {
		carFound = 0;
		sem_post(&driverSemaphore);
	}
	pthread_mutex_unlock(&lock);
	
	if(driver == 1) {
		sem_wait(&driverSemaphore);
		pthread_mutex_lock(&lock);
		printf("Thread ID: %ld, Team: %c, I am the captain and driving the car with ID %d\n", tid, Team, cars);
		cars++;
		
		if(queue >= 5){
			for(int i = 0; i < 5; i++) {
				sem_post(&inProgressSem);
			}
			queue -= 5;
		}
		else {
			for(int i = 0; i < queue; i++) {
				sem_post(&inProgressSem);
			}
			queue = 0;
		}
		
		inProgress = 0;
		pthread_mutex_unlock(&lock);
	}

	return NULL;
}

int main(int argc, char *argv[]) {

	if (pthread_mutex_init(&lock, NULL) != 0) { 
		printf("\n mutex init has failed\n");
		return 1;
    	}
    	
	ACount = atoi(argv[1]);
	BCount = atoi(argv[2]);
	
	pthread_t BIDs[BCount];
	pthread_t AIDs[ACount];

	sem_init( &ASemaphore , 0 ,0);
	sem_init( &BSemaphore , 0 ,0);
	
	char A = 'A';
	char B = 'B';
	
	if((ACount + BCount) % 4 == 0 && (ACount % 2 == 0) && (BCount % 2 == 0 )) {
		for(int i = 0; i < ACount; i++) {
			pthread_create(&AIDs[i], NULL, threadFunction, &A);
		}
		for(int i = 0; i < BCount; i++) {
			pthread_create(&BIDs[i], NULL, threadFunction, &B);
		}
		for(int i = 0; i < ACount; i++) {
			pthread_join(AIDs[i], NULL);
		}
		for(int i = 0; i < BCount; i++) {
			pthread_join(BIDs[i], NULL); 
		}
	}
	
	printf("The main terminates\n");
	
	pthread_mutex_destroy(&lock);
	sem_destroy(&ASemaphore);
	sem_destroy(&BSemaphore);
	sem_destroy(&driverSemaphore);
	sem_destroy(&inProgressSem);
	
	return 0;
}
