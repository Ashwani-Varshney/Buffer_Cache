#include "getblk.h"

getblk obj1;

//Newly created threads call this function for buffer allocation
void *processWantBuffers(void *threadarg)
{
    bool getbuff;
    struct thread_data *my_data;
    my_data = (struct thread_data *) threadarg;

    pthread_mutex_lock(&lock); 
    sleep(3);
    cout << "Thread ID : " << my_data->thread_id ;
    cout << " Block Number requested : " << my_data->blocknum << endl;
    getbuff = obj1.getblock(my_data->blocknum,my_data->thread_id);

    cout<<"\nTHREAD "<<my_data->thread_id<<" RELEASED BUFFER "<<my_data->blocknum<<"\n"; 
    displayFL();
    if(getbuff == true)          //Check if there are any waiting processes in common waiting list or in the current buffer's waiting list 
    { 
        pthread_cond_broadcast(&cond);    //Signal to wake up waiting processes
        pthread_cond_broadcast(&cond1);
    }       
    pthread_mutex_unlock(&lock);
    pthread_exit(NULL);
}

int main()
{
    init();         //Initialising the Data Structures used
    display();      //Displaying the initial state of Data Structures used
    pthread_mutex_init(&lock, NULL);
    pthread_t threads[8];
    struct thread_data td[8];
    srand(time(0));
    for( int i = 1; i<= 8; i++ ) 
    {
        pthread_mutex_lock(&lock); 
        cout <<endl<< "THREAD "<< i <<"  CREATED!!  "<<endl;
        sleep(1);
        pthread_mutex_unlock(&lock);
        td[i].thread_id = i;
        td[i].blocknum = (rand() % 13)+1;
        pthread_create(&threads[i], NULL, processWantBuffers, (void *)&td[i]);   
    }
    
    //Broadcasting signals to the pending threads
    int i= 1;
    while(i<=8)
    {
        if(!pthread_join(threads[i],NULL))
        {
            pthread_cond_broadcast(&cond);
            pthread_cond_broadcast(&cond1);
        }
        i++;
    }
    pthread_exit(NULL);
    return 0;   
}