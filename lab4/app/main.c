#include "lib.h"
#include "types.h"

void producer_consumer(){
	printf("Producer Consumer Problem\n");
	int i=0, ret=0;
	sem_t empty, full, mutex;
	sem_init(&empty, 5);
	sem_init(&full, 0);
	sem_init(&mutex, 1);
	for(i=0;i<4;i++){
		if(ret==0)ret = fork();
		else if(ret>0) break;
	}
	// 1 : consumer ; 2 3 4 5 : producer
	int id = getpid();
	if(id > 1){ //producer
		for(int i=0;i<2;i++){
			sem_wait(&empty); //emptyBuffers->P();
			sem_wait(&mutex); //mutex->P();
			printf("Producer %d: produce\n", id-1);
			sleep(128);
			sem_post(&mutex); //mutex->V();
			sem_post(&full); //fullBuffers->V();
		}
	}
	else if(id == 1){ //consumer
		for(int i=0;i<8;i++){
			sem_wait(&full); //fullBuffers->P();
			sem_wait(&mutex); //mutex->P();
			printf("Consumer : consume\n");
			sleep(128);
			sem_post(&mutex); //mutex->V();
			sem_post(&empty); //emptyBuffers->V();
		}
	}
	if(id!=1)exit();
	sem_destroy(&empty);
	sem_destroy(&full);
	sem_destroy(&mutex);
	exit();
}

void philosopher(){
	int i=0, ret=0;
	sem_t fk[5];
	for(int i=0;i<5;i++)sem_init(&fk[i], 1);
	for(i=0;i<4;i++){
		if(ret==0)ret = fork();
		else if(ret>0) break;
	}
	// philosopher 0-4, pid 1-5
	int id=getpid(); id-=1;
	for(int i=0;i<2;i++){
		printf("Philosopher %d: think\n", id);
		sleep(128);
		if(id%2==0){
			sem_wait(&fk[id]);
			sem_wait(&fk[(id+1)%5]);
		}
		else{
			sem_wait(&fk[(id+1)%5]);
			sem_wait(&fk[id]);
		}
		printf("Philosopher %d: eat\n", id);
		sleep(128);
		sem_post(&fk[id]);
		sem_post(&fk[(id+1)%5]);
	}
	if(id!=0)exit();
	for(int i=0;i<5;i++)sem_destroy(&fk[i]);
	exit();
}


// void reader_writer(){
// 	// 3 reader 3 writer
// 	// how to share the Rcount between processes?
// 	sem_t WriteMutex, CountMutex;
// 	// int Rcount = 0;
// 	sem_init(&WriteMutex, 1);
// 	sem_init(&CountMutex, 1);
// 	int i=0, ret=0;
// 	for(i=0;i<5;i++){
// 		if(ret==0)ret = fork();
// 		else if(ret>0) break;
// 	}
// 	// reader 1 2 3 writer 4 5 6
// 	int id = getpid();
// 	if(id>3){ //writer
// 		for(int i=0;i<2;i++){
// 			sem_wait(&WriteMutex);
// 			printf("Writer %d: write\n", id-3);
// 			sleep(128);
// 			sem_post(&WriteMutex);
// 		}
// 	}
// 	else { //reader
// 		for(int i=0;i<2;i++){
// 			sem_wait(&CountMutex);
// 			if(Rcount == 0)sem_wait(&WriteMutex);
// 			++Rcount;
// 			sem_post(&CountMutex);
// 			printf("Reader %d: read, total %d reader\n", id, Rcount); 
// 			sleep(128);
// 			sem_wait(&CountMutex);
// 			--Rcount;
// 			if(Rcount == 0)sem_post(&WriteMutex);
// 			sem_post(&CountMutex);
// 		}
// 	}
// 	if(id!=1)exit();
// 	sem_destroy(&WriteMutex);
// 	sem_destroy(&CountMutex);
// 	exit();
// }



int uEntry(void)
{

	// For lab4.1
	// Test 'scanf'
	// int dec = 0;
	// int hex = 0;
	// char str[6];
	// char cha = 0;
	// int ret = 0;
	// while (1)
	// {
	// 	printf("Input:\" Test %%c Test %%6s %%d %%x\"\n");
	// 	ret = scanf(" Test %c Test %6s %d %x", &cha, str, &dec, &hex);
	// 	printf("Ret: %d; %c, %s, %d, %x.\n", ret, cha, str, dec, hex);
	// 	if (ret == 4) 
	// 		break;
	// }

	// For lab4.2
	// Test 'Semaphore'
	// int i = 4;
	// int ret = 0;
	// sem_t sem;
	// printf("Father Process: Semaphore Initializing.\n");
	// ret = sem_init(&sem, 2);
	// if (ret == -1)
	// {
	// 	printf("Father Process: Semaphore Initializing Failed.\n");
	// 	exit();
	// }

	// ret = fork();
	// if (ret == 0)
	// {
	// 	while (i != 0)
	// 	{
	// 		i--;
	// 		printf("Child Process: Semaphore Waiting, i = %d\n", i);
	// 		sem_wait(&sem);
	// 		printf("Child Process: In Critical Area, i = %d\n", i);
	// 	}
	// 	printf("Child Process: Semaphore Destroying.\n");
	// 	sem_destroy(&sem);
	// 	exit();
	// }
	// else if (ret != -1)
	// {
	// 	while (i != 0)
	// 	{
	// 		i--;
	// 		printf("Father Process: Sleeping, i = %d\n", i);
	// 		sleep(128);
	// 		printf("Father Process: Semaphore Posting, i = %d\n", i);
	// 		sem_post(&sem);
	// 	}
	// 	printf("Father Process: Semaphore Destroying.\n");
	// 	sem_destroy(&sem);
	// 	exit();
	// }

	// For lab4.3
	// TODO: You need to design and test the problem.
	// Note that you can create your own functions.
	// Requirements are demonstrated in the guide.

	producer_consumer();
	// philosopher();
	// reader_writer();
	return 0;
}
