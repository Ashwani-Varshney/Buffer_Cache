#ifndef GETBLK_H
#define GETBLK_H
#include "initialize.h"


/*
Five typical scenarios the kernel may follow in getblk to allocate a buffer for a disk block.
	CASE 1: The kernel finds the block on the hash queue, but its buffer is currently busy.
	CASE 2: The kernel finds the block on its hash queue, and its buffer is free.
	CASE 3: The kernel cannot find the block on the hash queue, and the free list of buffers is empty.
	CASE 4: The kernel cannot find the block on the hash queue, so it allocates a buffer from the free list.
	CASE 5: The kernel cannot find the block on the hash queue and, in attempting to allocate a buffer from the free list 
			(as in scenario 2),finds a buffer on the free list that has been marked "delayed write". The kernel must write the 
			"delayed write" buffer to disk and allocate another buffer.
*/


class getblk
{

public:

    void case1(buffer* buff, int pnum)
    {
        cout<<"CASE 1 :Buffer in HASH QUEUE but not in FREELIST\n";
        //pthread_cond_broadcast(&cond);
        waitingLists[buff->blockNumber]->insertProcessAtWaitingListTail(pnum);
        buff->in_demand=true;
        cout<<"THREAD "<<pnum<<" IS GOING TO SLEEP ...\n\n";
	    sleep(2);
	    cout<<"THREAD INSERTED INTO WAITING LIST .. \n";
        displayWL();
        pthread_cond_wait (&cond1, &lock);          // Waits for signal
    }

    bool case2(buffer* buff, int pnum,int flag)
    {   
        pthread_mutex_lock(&lock1);
        bool f = 1;
        cout<<"CASE 2 :Buffer in HASH QUEUE and FREELIST"<<"\n";
        int hashv;
        if(flag==4 || flag==5|| flag==3)	// If case 2 is called from case 3, 4 or 5 
        {
            hashv=hashValue(buff->blockNumber);
            hashQueueLists[hashv]->insertBufferAtHashQueueTail(buff);
        }
		if(flag==2||flag==3)				// If case 2 is called from case 3 or directly from getblock
           freeList->removeBufferFreeList(buff);
	
 		buff->lock= true;
        bool check_waitingList=waitingLists[buff->blockNumber]->is_waitingList_empty();
        bool check_Common_waitingList=  waitingLists[14]->is_waitingList_empty();
        if(check_waitingList && check_Common_waitingList)		
            buff->in_demand=false;
        else
            buff->in_demand=true;
        display();
		buff->lock=false;
        if (buff->blockNumber==8||buff->blockNumber==2||buff->blockNumber==6)  // Setting delayedWrite = true for some buffers
        {
            buff->delayedWrite=true;
        }
        freeList->insertBufferAtFreeListTail(buff);
        if(buff->in_demand==true)       
            f = true;
        pthread_mutex_unlock(&lock1);
        return f;
    }

    void case3(int pnum)
    {
        pthread_mutex_lock(&setl);
        cout<<"CASE 3 : Buffer not in HASH QUEUE and FREELIST empty"<<"\n";
        waitingLists[14]->insertProcessAtWaitingListTail(pnum); 
        for(int i=0;i<NUM_OF_QUEUE;i++) 
            hashQueueLists[i]->setInDemandInHashQueue();
        cout<<"THREAD "<<pnum<<" IS GOING TO SLEEP ...\n\n";
		sleep(2);
		cout<<"THREAD INSERTED INTO WAITING LIST .. \n";
        displayWL();
        pthread_mutex_unlock(&setl);
	    pthread_cond_wait (&cond, &lock);           // Waits for signal
    } 

    bool case4(buffer* buff,int blkno,int pnum)
    {   
    	bool a,b; 
        cout<<"CASE 4 : Buffer at head in FREELIST without Delayed Write"<<"\n";
	    b=hashQueueLists[hashValue(buff->blockNumber)]->removeBufferHashQueue(buff->blockNumber);
        buff->blockNumber=blkno;
        a = case2(buff,pnum,4);
        return a;
    }

    bool case5(buffer* buff,int pnum, int blkno)
    {
  		bool c;
        int f=5;
        listOfbuff = new bufferCache();
		while(buff->delayedWrite==true)
	   	{
	   		cout<<"\nTHREAD "<<pnum<<" PERFORMING ASYNCHRONOUS WRITE FOR BLOCK NUMBER "<<buff->blockNumber;
	   		cout<<"\nJUMPING TO NEXT BUFFER IN FREELIST"<<endl;
            if(freeList->isEmpty())
            {
		      	f=3;
                buff->delayedWrite=false;
                freeList->insertBufferAtHeadFreeList(buff);
                case3(pnum);
				cout<<endl<<" THREAD  "<<pnum<<" RESUMES EXECUTION ........"<<"\n";
				waitingLists[14]->removeProcessFromWaitingList(pnum);
				c=getblock(blkno,pnum);
				return c;
            }
            else
            {
                sleep(5);
                cout<<"\n";
                buff->delayedWrite=false;
                listOfbuff->insertBufferAtFreeListTail(buff);
                buff=freeList->removeBufferFromHeadFreeList();
            }
        }
        while(listOfbuff->isEmpty()==false)
        {
            buffer* bf = listOfbuff->removeBufferFromHeadFreeList();
            freeList->insertBufferAtHeadFreeList(bf);
        }
	   	bool b=hashQueueLists[hashValue(buff->blockNumber)]->removeBufferHashQueue(buff->blockNumber);
        buff->blockNumber=blkno;
        c=case2(buff,pnum,f);
        return c;
    }

	bool getblock(int blkno,int pnum)
	{
        int counter = 0;  //Maintaining a counter to release buffers in case 1
    	callagain :
        	bool getbuf = false;	
        	int hv=hashValue(blkno);
        	buffer* foundBuffer=hashQueueLists[hv]->findBufferHashQueue(blkno);
        	if(foundBuffer)
        	{
            	if(foundBuffer->lock)
            	{
                	case1(foundBuffer,pnum);
                	counter++;                            
                	pthread_mutex_lock(&setlock); 
		          	sleep(3);
                	if(counter == 1) //Checking counter and releasing buffer which were locked in the initial configuration
                	{
                    	foundBuffer->lock = false;
		    			cout<<endl<<endl<<"RELEASED BUFFER : "<<foundBuffer->blockNumber;
		    			buffer* h=freeList->insertBufferAtFreeListTail(foundBuffer);
		    			cout<<"\n-----------------------------------------------------------------------------------\n";
		    			displayFL();
    		    		cout<<endl<<"\n THREAD  "<<pnum<<" RESUMES EXECUTION........"<<"\n";
                    	waitingLists[blkno]->removeProcessFromWaitingList(pnum);
                	}
					pthread_mutex_unlock(&setlock); 
                	goto callagain;        //Thread calls getblock again  
           		}
            	else if(foundBuffer->delayedWrite)
	       		{
		      		foundBuffer->delayedWrite=false;
		      		freeList->removeBufferFreeList(foundBuffer);
		      		cout<<"CASE 5 : Buffer in HASH QUEUE and FREELIST with Delayed Write"<<"\n";
		      		cout<<"\nTHREAD "<<pnum<<" PERFORMING ASYNCHRONOUS WRITE FOR BLOCK NUMBER "<<foundBuffer->blockNumber;
	          		getbuf = case5(foundBuffer,pnum,blkno);
	       		}                        
	       		else
	        		getbuf = case2(foundBuffer,pnum,2);
	        }
        	else
        	{
            	bool freelistEmpty=freeList->isEmpty();
            	if(freelistEmpty)
           		{
					pthread_mutex_init(&setlock1, NULL);
                	pthread_mutex_lock(&setlock1);
                	case3(pnum);
  		        	sleep(3);
                	cout<<endl<<" THREAD  "<<pnum<<" RESUMES EXECUTION........"<<"\n";
					waitingLists[14]->removeProcessFromWaitingList(pnum);
					pthread_mutex_unlock(&setlock1); 
                	goto callagain;         //thread calls getblock again
            	}
            	else
            	{
                	buffer* buff = freeList->removeBufferFromHeadFreeList();
                	bool dw= buff->delayedWrite;
                	if(dw)
		  			{
		     			cout<<"CASE 5 : Buffer in FREELIST with Delayed Write"<<"\n";
                     	getbuf = case5(buff,pnum,blkno);
		  			}
                	else
                   		getbuf = case4(buff,blkno,pnum);
            	}
                
        	}
    	return getbuf;
	}
}obj;

#endif //GETBLK_H