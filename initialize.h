#ifndef INITIALIZE_H
#define INITIALIZE_H
#include "bufferCache.h"

#define NUM_OF_QUEUE 4
#define NUM_OF_BUFFERS 6
#define NUM_OF_WL 14    //Assuming there are 13 blocks being demanded by threads 
                        // Waiting list-14 represents the common waiting list

bool specificBuffer = false; 
bool anyBuffer = false;
int *MEMORY;

bufferCache **hashQueueLists;
bufferCache *freeList;
bufferCache *listOfbuff;
waitingList **waitingLists;

pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t lock1 = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t setl = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t lockwait = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t setlock = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t setlock1 = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t buf_cond = PTHREAD_COND_INITIALIZER;
pthread_cond_t cond = PTHREAD_COND_INITIALIZER;
pthread_cond_t cond1 = PTHREAD_COND_INITIALIZER;

struct thread_data
{
   int thread_id;
   int blocknum;
   int thread_num;
};

int hashValue(int blockNumber)
{
    int hashValue = 0;
    hashValue = blockNumber % NUM_OF_QUEUE;
    return hashValue;
}

void init()
{
   
    //Hash queue intialisation
    hashQueueLists = new bufferCache*[NUM_OF_QUEUE];
    for(int i = 0;i < NUM_OF_QUEUE;i++)
    {
        hashQueueLists[i] = new bufferCache();
    }

    //Insert new buffers in the hash queue 
    for (int i=1; i <= NUM_OF_BUFFERS; i++)
        hashQueueLists[hashValue(i)]->insertBufferAtHashQueueTail(new buffer(i)); 

    //Free List initialisation
    buffer* inserted;
    freeList = new bufferCache();
    inserted = freeList->insertBufferAtFreeListTail(new buffer());
    inserted->blockNumber = 3;  //Initially block number-3 is unlocked, hence available in Freelist

    //Waiting List initialisation
    waitingLists = new waitingList*[NUM_OF_WL];      
    for(int i=1;i<=NUM_OF_WL;i++)
    	waitingLists[i]=new waitingList();
}

void displayHQ()
{
	sleep(1);
	cout<<"\n\nHashQueue Details ";
    for(int i = 0; i < NUM_OF_QUEUE; i++)
    {
        cout<<"\nHashQueue "<<i<<" : ";
        hashQueueLists[i]->displayHashQueue();
    }	
}

void displayFL()
{
	sleep(1);
	cout<<"\n-----------------------------------------------------------------------------------\n";
    cout<<"\nFreeList Details \n";
    freeList->displayFreeList();
    cout<<"\n----------------------------------------------------------------------------------- \n";
}

void displayWL()
{
	sleep(1);
	cout<<"\nWaitingList Details\n";
    for(int i = 1;i <= NUM_OF_WL-1;i++)
    {
        cout<<"\nWL "<<i<<" : ";
        waitingLists[i]->displayWaitingList();
    }
    cout<<"\nCommon WL "<<" : ";
    waitingLists[14]->displayWaitingList();
    cout<<"\n-----------------------------------------------------------------------------------\n";

}
void display()
{   
    displayHQ();
    displayFL();
    displayWL(); 
    sleep(1);
}
#endif //WAITINGLIST_H