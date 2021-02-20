#include <iostream>
#include <pthread.h>
#include <stdio.h>
#include <unistd.h>
#include <string>
#include <stdlib.h> 
#include <queue> 
#include <semaphore.h>
using namespace std;

#define NUM_THREADS 5
#define MEMORY_SIZE 1000

struct node
{
	int id;
	int size;
};

pthread_t thread[NUM_THREADS];
bool Release = false;
queue<node> myqueue; // shared que
pthread_mutex_t sharedLock = PTHREAD_MUTEX_INITIALIZER; // mutex
pthread_t server; // server thread handle
sem_t semlist[NUM_THREADS]; // thread semaphores

int thread_message[NUM_THREADS]; // thread memory information
char  memory[MEMORY_SIZE]; // memory size




void release_function()
{
	//This function will be called
	//whenever the memory is no longer needed.
	//It will kill all the threads and deallocate all the data structures.
	
	// deleting queue, memory array and semaphores
	for (int i = 0; i < NUM_THREADS; i++)
	{
		sem_destroy(&semlist[i]);
	}
	while (!myqueue.empty())
	{
		myqueue.pop();
	}

	for (int i = 0; i < NUM_THREADS; i++)
	{
		pthread_cancel(thread[i]);
	}
	delete[] memory;
	delete[] thread;
}

void my_malloc(int thread_id, int size)
{

	//This function will add the struct to the queue
	node tobeadded;
	tobeadded.id = thread_id;	
	tobeadded.size = size;

	myqueue.push(tobeadded);
	
}

void * server_function(void *)
{

	//This function should grant or decline a thread depending on memory size.
	int loc = 0;
	while (!Release)
	{
		pthread_mutex_lock(&sharedLock);
		if (!myqueue.empty())
		{
			node temp = myqueue.front();
			myqueue.pop();
			int tid = temp.id;
			int tsize = temp.size;
			if (MEMORY_SIZE - loc >= tsize)
			{
				thread_message[tid] = loc;
				loc += tsize;
			}
			else
			{
				thread_message[tid] = -1;
			}
			sem_post(&semlist[tid]);
		}
		pthread_mutex_unlock(&sharedLock);
	}
	pthread_exit(NULL);
}

void * thread_function(void * id) 
{
	//This function will create a random size, and call my_malloc
	int *tid = (int *) id;
	int RandSize = (rand() % 1000 + 1);
	my_malloc(*tid, RandSize);
	//Block
	sem_wait(&semlist[*tid]);
	int rVal = thread_message[*tid];
	//Then fill the memory with 1's or give an error prompt
	if (rVal != -1)
	{
		for (int i = 0; i < RandSize; i++)
		{
			memory[i + rVal] = '1';
		}
	}
	else
	{
		cout << "Thread" << *tid << ": not enough memory" << endl;
	}
	pthread_exit(NULL);
}

void init()	 
{
	pthread_mutex_lock(&sharedLock);	//lock
	for(int i = 0; i < NUM_THREADS; i++) //initialize semaphores
	{sem_init(&semlist[i],0,0);}
	for (int i = 0; i < MEMORY_SIZE; i++)	//initialize memory 
  	{char zero = '0'; memory[i] = zero;}
   	pthread_create(&server,NULL,server_function,NULL); //start server 
	pthread_mutex_unlock(&sharedLock); //unlock
}



void dump_memory() 
{
 // You need to print the whole memory array here.
	cout << "Memory Dump:" << endl;
	for (int i = 0; i < MEMORY_SIZE; i++)
	{
		cout << memory[i];
	}
}

int main (int argc, char *argv[])
 {
	srand(time(NULL));
	//You need to create a thread ID array here
	int thrdid[NUM_THREADS];
	for (int i = 0; i < NUM_THREADS; i++)
	{
		thrdid[i] = i;
	}

 	// call init
	init();

 	//You need to create threads with using thread ID array, using pthread_create()
	for (int i = 0; i < NUM_THREADS; i++)
	{
		//cout << i << endl;
		pthread_create(&thread[i], NULL, thread_function, &thrdid[i]);
	}
 	//You need to join the threads
	for (int i = 0; i < NUM_THREADS;i++)
	{
		pthread_join(thread[i],NULL);
	}
	Release = true;
	pthread_join(server, NULL);
 	dump_memory(); // this will print out the memory
 	printf("\nMemory Indexes:\n" );
 	for (int i = 0; i < NUM_THREADS; i++)
 	{
 		printf("[%d]" ,thread_message[i]); // this will print out the memory indexes
 	}
 	printf("\nTerminating...\n");
	release_function();
 }