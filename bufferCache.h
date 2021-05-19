#ifndef BUFFERCACHE_H
#define BUFFERCACHE_H
#include <string.h>
#include <iostream>
#include <cstdlib>
#include <pthread.h>
#include <unistd.h>
#include <signal.h>

using namespace std;

pthread_mutex_t lockinsert = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t lockFL = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t lockHQ = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t lockgen1 = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t lockgen2 = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t lockgen3 = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t lockgen4 = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t lockgen5 = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t lockgen6 = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t lockgen7 = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t lockgen8 = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t lockgen9 = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t lockgen10 = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t lockgen11 = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t lockgen12 = PTHREAD_MUTEX_INITIALIZER;


struct buffer
{
    int device_num;
    int blockNumber;
    bool lock;
    bool valid_data;
    bool delayedWrite;
    bool in_demand;
    bool read;
    bool write;

    struct buffer* nextHashQueue;
    struct buffer* prevHashQueue;
    struct buffer* nextFreeList;
    struct buffer* prevFreeList;

public:

    buffer(int blockNumber = 0)
    {
        this->blockNumber = blockNumber;
        device_num = 0;
        lock = false;
        delayedWrite = false;
        valid_data = false;
        in_demand = false;
        read = false;
        write= false;
        nextHashQueue = NULL;
        prevHashQueue = NULL;
        nextFreeList = NULL;
        prevFreeList = NULL;
    }

};


class bufferCache
{

    buffer* head;
    buffer* tail;

public:

    //Check if HashQueue or FreeList is Empty or Not
    bool isEmpty()
    {
        return (head == NULL);
    }

    buffer* getHead()
    {
        return head;
    }

    buffer* getTail()
    {
        return tail;
    }

    //check if there is any process waiting for a buffer in specific or common waiting list, set in_demand accordingly
    void setInDemandInHashQueue()
    {
        pthread_mutex_lock(&lockgen1);
    	if (!isEmpty())
    	{
	   		buffer* temp=head; 
	   		do
        	{   
        		if(temp->lock==true)
	       		{
	 	       		temp->in_demand=true;
	       		}
            	temp=temp->nextHashQueue;
        	}while(temp!= head);
    	}
        pthread_mutex_unlock(&lockgen1);
	}

	//Find a Buffer in HashQueue in the bufferCache
    buffer* findBufferHashQueue(int blockNumber)
    {
        pthread_mutex_lock(&lockgen2);
        buffer* foundBuffer = NULL;
        if(!isEmpty())
        {
            buffer* temp = head;
            do{
                if(temp->blockNumber == blockNumber)
                {
                    foundBuffer = temp;
                    break;
                }
                temp = temp->nextHashQueue;
            }while(temp != head);
        }
        pthread_mutex_unlock(&lockgen2);
        return foundBuffer;
    }

    //Find a Buffer in FreeList in the bufferCache
    buffer* findBufferFreeList(int blockNumber)
    {
        pthread_mutex_lock(&lockgen3);
        buffer* foundBuffer = NULL;
        if(!isEmpty())
        {
            buffer* temp = head;
            do{
                if(temp->blockNumber == blockNumber)
                {
                    foundBuffer = temp;
                    break;
                }
                temp = temp->nextFreeList;
            }while(temp != head);
        }
        pthread_mutex_unlock(&lockgen3);
        return foundBuffer;
    }

 
    //Add Buffer at Start of the Free List in the bufferCache
    void insertBufferAtHeadFreeList(buffer* insertBuffer)
    {
        pthread_mutex_lock(&lockgen4);
        if(head == NULL)
        {
            insertBuffer->nextFreeList = insertBuffer->prevFreeList = insertBuffer;
            head = tail = insertBuffer;
	        insertBuffer->lock=false;
        }
        else
        {
            insertBuffer->nextFreeList = head;
            insertBuffer->prevFreeList = tail;
            tail->nextFreeList = head->prevFreeList = insertBuffer;
            head = insertBuffer;
	        insertBuffer->lock=false;
        }
        pthread_mutex_unlock(&lockgen4);
    }

    //Add Buffer at End of the FreeList in bufferCache
    buffer* insertBufferAtFreeListTail(buffer* insertBuffer)
    {   
        pthread_mutex_lock(&lockinsert);
        if(head == NULL)
        {   
            insertBuffer->nextFreeList = insertBuffer->prevFreeList = insertBuffer;
            head = tail = insertBuffer;
            insertBuffer->lock=false;
        }
        else
        {   
            insertBuffer->prevFreeList = tail;
            insertBuffer->nextFreeList = head;
            head->prevFreeList = tail->nextFreeList = insertBuffer;
            tail = tail->nextFreeList;
	    tail->nextFreeList=head;
	    head->prevFreeList=tail;
            insertBuffer->lock=false;
        }
        pthread_mutex_unlock(&lockinsert);
        return insertBuffer;
        
    }


    //Add Buffer at End of the HashQueue in bufferCache
    void insertBufferAtHashQueueTail(buffer* insertBuffer)
    {
        pthread_mutex_lock(&lockgen5);
	    buffer* temp=head;
        if(head == NULL)
        {
            insertBuffer->nextHashQueue = insertBuffer->prevHashQueue = insertBuffer;
            head = tail = insertBuffer;
            if(insertBuffer->blockNumber == 3)
                insertBuffer->lock=false;
            else 
                insertBuffer->lock=true;
        }
        else
        {
            insertBuffer->prevHashQueue = tail;
            insertBuffer->nextHashQueue = tail->nextHashQueue;
            head->prevHashQueue = tail->nextHashQueue = insertBuffer;
            tail = tail->nextHashQueue;
	        if(insertBuffer->blockNumber == 3)
                insertBuffer->lock=false;
            else 
                insertBuffer->lock=true;
        }
        pthread_mutex_unlock(&lockgen5);
    }

	//Remove Buffer from Head of the FreeList
    buffer* removeBufferFromHeadFreeList()
    {
        pthread_mutex_lock(&lockgen6);
        buffer* removed = NULL;
        if(!isEmpty())
        {
            if(head == head->nextFreeList)  //Only Buffer in the FreeList
            {
                removed = head;
                head = tail = NULL;
            }
            else
            {
                tail->nextFreeList = head->nextFreeList;
                head->nextFreeList->prevFreeList = head->prevFreeList;
                removed = head;
                head = head->nextFreeList;
            }
        }
        pthread_mutex_unlock(&lockgen6);
        return removed;
    }

	//Remove Buffer from Tail of the FreeList
    buffer* removeBufferFromTailFreeList()
    {
        pthread_mutex_lock(&lockgen7);
        buffer* removed = NULL;
		buffer* temp = tail;
        if(!isEmpty())
        {
			tail->prevFreeList->nextFreeList = head;
			head->prevFreeList = tail->prevFreeList;
            tail=tail->prevFreeList;
        }
        pthread_mutex_unlock(&lockgen7);
        return removed;
    }

	//Remove a particular Buffer from HashQueue in the bufferCache
    bool removeBufferHashQueue(int blockNumber)
    {
        pthread_mutex_lock(&lockgen8);
        bool removed = false;
        buffer* temp = findBufferHashQueue(blockNumber);
        if(temp != NULL)
        {
            removed = true;
            if(temp->nextHashQueue == temp)		//Only Buffer in the HashQueue in bufferCache
            {       
                head = tail = NULL;
            }
            else
            {
                if(temp == head)	//If Head is to be removed from HashQueue in bufferCache
                {               
                    head = head->nextHashQueue;
                }
                else if(temp == tail)	//If Tail is to be removed from HashQueue in bufferCache
                {         
                    tail = tail->prevHashQueue;
                }
                temp->prevHashQueue->nextHashQueue = temp->nextHashQueue;
                temp->nextHashQueue->prevHashQueue = temp->prevHashQueue;
            }
        }
        pthread_mutex_unlock(&lockgen8);
        return removed;
    }

	//Remove a particular Buffer from FreeList in the bufferCache
    void removeBufferFreeList(buffer* buff)
    { 
        pthread_mutex_lock(&lockgen9);
        buffer* foundBuffer = NULL; 
		if(!isEmpty())
		{ 	
            buffer* temp = head;
	    	if(head->blockNumber == buff->blockNumber)
			{   
		    	buffer* b=removeBufferFromHeadFreeList();
	        }
	    	else if(tail->blockNumber == buff->blockNumber)
			{   
		    	buffer* b=removeBufferFromTailFreeList();
	        }
            else
            {
				do
				{ 
               		if(temp->blockNumber == buff->blockNumber)
               		 {   
                    		temp->prevFreeList->nextFreeList=temp->nextFreeList;
                    		temp->nextFreeList->prevFreeList = temp->prevFreeList;	
                    		temp->nextFreeList = NULL;
                    		temp->prevFreeList = NULL;
                    		break;
               		 }
                	temp = temp->nextFreeList;
            	}while(temp != head);
        	} 
    	}
        pthread_mutex_unlock(&lockgen9);
    }

    
    void displayFreeList()
    {
        pthread_mutex_lock(&lockFL);
        buffer* temp = head;
        if(!isEmpty())
        {
            do
            {
                cout<<"{"<<temp->blockNumber<<", ";
                cout<<(temp->lock ? "L" : "UL")<<", "<<(temp->delayedWrite ? "DW" : "NDW")<<"}\t";
                temp = temp->nextFreeList;
            }while(temp != head);
        }
        else
            cout<<"Empty List";
        pthread_mutex_unlock(&lockFL);
    }

    void displayHashQueue()
    {
        pthread_mutex_lock(&lockHQ);
        buffer* temp = head;
        if(!isEmpty())
        {
            do
            { 
                cout<<"{"<<temp->blockNumber<<", "<<(temp->in_demand? "ID" : "NID")<<", ";
                cout<<(temp->lock ? "L" : "UL")<<", "<<(temp->delayedWrite ? "DW" : "NDW")<<"}     ";
                temp = temp->nextHashQueue;
            }while(temp != head);
        }
        else
            cout<<"Empty List";
        pthread_mutex_unlock(&lockHQ);
    }
};

struct waitnode
{
    int pno; 
    waitnode* nextWaitingPtr;
};

class waitingList 
{
    waitnode* whead=NULL;
    waitnode* wtail=NULL;
    
public:
    
	//Add Buffer at End of the WaitingList
    void insertProcessAtWaitingListTail(int pnum)
    {
        pthread_mutex_lock(&lockgen10);
		waitnode* wnode = new waitnode();
        if(whead == NULL)
        {
	    	whead=wnode;
            wnode->nextWaitingPtr = NULL;
            wnode->pno = pnum;
	    	wtail=wnode;
        }
        else
        {
            wtail->nextWaitingPtr = wnode;
            wnode->pno=pnum;
	    	wnode->nextWaitingPtr = NULL;
            wtail = wnode;
        }
        pthread_mutex_unlock(&lockgen10);
    }

	bool is_waitingList_empty()
	{
        if(whead==NULL)
            return true;
        else
            return false;
    }

    void removeProcessFromWaitingList(int pnum)
    {
        pthread_mutex_lock(&lockgen11);
        waitnode* temp2 = whead;
        waitnode* temp1 = whead;
        if(whead!=NULL)
        {   
            if(whead->pno==pnum)
                whead=whead->nextWaitingPtr;
            else
            {
                temp2=temp2->nextWaitingPtr;
                do
                {   
                    if(temp2->pno==pnum)
                        {
                            temp1->nextWaitingPtr=temp2->nextWaitingPtr;
                            temp2->nextWaitingPtr=NULL;
                            break;
                        }
                    temp1=temp1->nextWaitingPtr;
                    temp2=temp2->nextWaitingPtr;    
                }while(temp2->nextWaitingPtr!=NULL);
            }
        }
        pthread_mutex_unlock(&lockgen11);
    }

	//Display waiting list
    void displayWaitingList()
    {
        pthread_mutex_lock(&lockgen12);
        waitnode* temp = whead;
        if(!(whead==NULL))
        {
            do
            {
                cout<<"{"<<temp->pno<<" } ";
                temp = temp->nextWaitingPtr;
            }while(temp != NULL);
        }
        else
            cout<<"Empty List";
        pthread_mutex_unlock(&lockgen12);
    }
};

#endif // BUFFERCACHE_H