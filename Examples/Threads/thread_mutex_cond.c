#include<pthread.h>
#include<stdio.h>
#include<stdlib.h>
#include<time.h>
#include<malloc.h>
#include<unistd.h>
#include<string.h>
#include<semaphore.h>

//in this example, we are using mutexes and condition variables to 
//  implement a message queue in user space - 2 threads are 
//  accessing the message queue,
//  in user-space - thread2 is producer thread and thread1 is 
//    the consumer thread both have to take of data exchange, 
//  synchronization and critical section !!!




/* 0COMPILE WITH THE FOLLOWING COMMAND 
	gcc -Wall -D_GNU_SOURCE mutex_simple.c -o mutex -pthread

	vi  /usr/include/features.h 
/////////////////////////////////////////////////////////// */

/* If _GNU_SOURCE was defined by the user, turn on all the other features.  
#ifdef _GNU_SOURCE
# undef  _ISOC99_SOURCE
# define _ISOC99_SOURCE 1
# undef  _POSIX_SOURCE
# define _POSIX_SOURCE  1
# undef  _POSIX_C_SOURCE
# define _POSIX_C_SOURCE        200112L
# undef  _XOPEN_SOURCE
# define _XOPEN_SOURCE  600
# undef  _XOPEN_SOURCE_EXTENDED
# define _XOPEN_SOURCE_EXTENDED 1
# undef  _LARGEFILE64_SOURCE
# define _LARGEFILE64_SOURCE    1
# undef  _BSD_SOURCE
# define _BSD_SOURCE    1
# undef  _SVID_SOURCE
# define _SVID_SOURCE   1
# undef  _ATFILE_SOURCE
# define _ATFILE_SOURCE 1
#endif

//////////////////////////////////////////////////////

	vi /usr/include/pthread.h 
 
///////////////////////////

{
  PTHREAD_MUTEX_TIMED_NP,
  PTHREAD_MUTEX_RECURSIVE_NP,
  PTHREAD_MUTEX_ERRORCHECK_NP,
  PTHREAD_MUTEX_ADAPTIVE_NP
#ifdef __USE_UNIX98
  ,
  PTHREAD_MUTEX_NORMAL = PTHREAD_MUTEX_TIMED_NP,
  PTHREAD_MUTEX_RECURSIVE = PTHREAD_MUTEX_RECURSIVE_NP,
  PTHREAD_MUTEX_ERRORCHECK = PTHREAD_MUTEX_ERRORCHECK_NP,
  PTHREAD_MUTEX_DEFAULT = PTHREAD_MUTEX_NORMAL
#endif

//////////////////////////////////////////////////////////

*/

//a mutex object must be a global object in the process
//a mutex object is also an abstract object of the thread library 
//initialize and use it using thread APIs 

//in user-space !!
pthread_mutex_t m1;   //instantiate as many as you need !!!

	//create a object of mutex

//sem_t sem1;

pthread_cond_t eq_cv1,dq_cv2;  //enqueue and dequeue

pthread_t th_id1,th_id2;// two threads from pthread

//void cleanup1(void *arg1)

//message queue object - can be customized as per our development
//requirements - in this example, it is just a basic structure -
//using mutexes and condition variables !!! 

//what will be the working of the threads, if mutex is used
//and condition variables are not used !!! we will use appropriate
//conditional checks using certain maintenance information !!!


//consumer will end up in a busy waiting while loop -
//this leads to an inefficient thread and it is
//unacceptable in a multithreaded application !!!


//what is the alternative or other mechanism to be added ???


//Note: do refer to class pseudo code for analysis !!!



struct job_head { 
      unsigned int queue_count;
      struct job *next;

}job_head1;


struct job
{
	char str1[2048];
	struct job *next;
};

//struct job* job_queue;

//receiver or consumer
void *thread_func1(void *arg)
{
	while(1)
	{
		struct job* next_job;

        	pthread_mutex_lock(&m1);
            
                //use as many condition variables as the no of conditions
                //use as many mutexes as the no of sets of related critical sections
                //what happens, if the following condition is 
                //true ???
                //current thread will be blocked in the wq of 
                //dq_cv2 condition variable and in addition,
                //as per the rules of condition variable operations,
                //the associated mutex is unlocked !!!   

                while(job_head1.queue_count < 5)
                       pthread_cond_wait(&dq_cv2,&m1);

                //let us assume that some time in the future,
                //condition is false and another thread has 
                //woken up this thread - this thread will wake up 
                //in the middle of pthread_cond_wait() and
                //as per the rules of the condition variable operations,
                //will implicitly lock the associated mutex and
                //resume the execution !!!

                while(1){
			if(job_head1.next == NULL)
			{
				next_job = NULL;
                                break;
			}
			else
			{
				next_job = job_head1.next;
				job_head1.next = job_head1.next->next;
                        

                        //used here for condition variable and 
                        //statistics !!!
                       	job_head1.queue_count--;
     printf("in dequeue count is %d\n", job_head1.queue_count);
     printf("the string is %s\n", next_job->str1);
                                free(next_job);
                         
                	}   
		  }//inner-while

               
                //after the receiver thread consumes all messages
                //from the message queue, it is bound to wake up 
                //the other condition variable of sender 
                //condition is always true, when we reach here !!!
                pthread_cond_signal(&eq_cv1);                



                //unlocking mutex is also part of the semantics
		pthread_mutex_unlock(&m1);

	}//outer-while
	pthread_exit(NULL);
	return(NULL);
}

//sender or producer

void *thread_func2(void *arg)
{
	struct job *new_job;
//	new_job = (struct job *)malloc(sizeof(struct job));
         
	while(1)
	{
                //we are locking the mutex and entering the 
                //critical section !!!
            	pthread_mutex_lock(&m1);

                //check whether the message queue has 5 or mor e
                //messages 
                //if true, unconditionally block the current thread
                //in the wq of th e
                //condition variable by invoking 
                //pthread_cond_wait() !!!                
                //pthread_cond_wait() works specially 
                //meaning, when it blocks the current thread
                //in the wq of the cond variable, it will also 
                //unlock the associated mutex, automatically 
                //in short, we block the current thread, but 
                //also release the associated mutex !!!
                //this thread will be woken by another thread 
                //some time in the future, when the other
                //thread uses pthread_cond_signal() on the 
                //condition variable !!!
                //when this thread wakes up, it will resume
                //from the middle of pthread_cond_wait() 
                //after resuming, pthread_cond_wait() will 
                //implicitly lock the mutex and return - once again,
                //pthread_cond_wait() is peculiar !!!

                //if you analyse condition variable's working, 
                //you will find that it is very versatile for 
                //a developer - use it along with mutex locks!!!

                while(job_head1.queue_count >= 5)  //add the condition
                     pthread_cond_wait(&eq_cv1,&m1);
//malloc can provide smaller chunks of memory
//mmap() can only provide memory chunks that are multiple of page size                  
  new_job = (struct job *)malloc(sizeof(struct job));   
	
 read(STDIN_FILENO,new_job->str1,sizeof(new_job->str1)); //take user input in a local buffer
    
//		printf("Enter the string to be Queued \n");
                
	//	gets(new_job->str1);
        //	strcpy(new_job->str1,"ravivek");
		if(job_head1.next == NULL)
                {
                    job_head1.next = new_job;
                    new_job->next = NULL;
                }

                else{
                     struct job *temp = job_head1.next;
                     while(temp->next != NULL)
                            temp = temp->next;
                     temp->next = new_job;
                     new_job->next = NULL;
                }         


                //such counters are predominantly statistical !!
                //however, in this context , it is really used
                //reason, condition variable uses such variables !!!
                job_head1.queue_count++;

                printf("in enqueue count is %d\n", job_head1.queue_count);

    

               //we come back here after one run !!!
             
               //if this condition is true(regarding receiver thread),
               //the receiver thread will be woken up, if it is
               //blocked in the wq of the dq_cv2!!!

               if(job_head1.queue_count >=5) 
                         pthread_cond_signal(&dq_cv2);

               //as per the rules of the condition variable operations,
               //we are expected to release the lock after 
               //signalling a condition variable - 
               //otherwise, the other thread will never be 
               //able to progress !!!!

       
		pthread_mutex_unlock(&m1); //after wake up of a condition 
                                           //variable queue, release the mutex
                                           //otherwise, other thread will be 
                                           //blocked in the mutex, after wake-up 
                                           //from condition variable - refer to
                                           //semantics of condition variable !!!

                //sem_post(&sem1);

	//	sched_yield();    //you may use, if needed !!!

	}
	pthread_exit(NULL);
}



int main()
{
	int ret1,ret2;


        //mutexes also have attributes and attribute objects
        //most initialization rules are similar to that of thread creation !!!


	pthread_mutexattr_t ma1;
	pthread_attr_t tha1,tha2;
	int policy1,polciy2;
	struct sched_param p1;


        job_head1.queue_count = 0;
        job_head1.next = NULL;
	//alarm(300);
	printf("I am in Main Thread and PID is %lu .. PPID is %lu\n",getpid(),getppid());
        //ptr to semaphore object
        //process shared field - 0, if it is PROCESS_PRIVATE
        //                       1, if it is PROCESS_SHARED  
        //the initial value of the semaphore count = 0
        //sem_init(&sem1,0,0);

	pthread_attr_init(&tha1); // initialize attribute object - default
	pthread_mutexattr_init(&ma1);//initialize to default attributes !!

        //refer to manual page of pthread_mutexattr_settype() for 
        //different types of mutexes !!!

	//pthread_mutexattr_settype(&ma1,PTHREAD_MUTEX_NORMAL);
	pthread_mutexattr_settype(&ma1,PTHREAD_MUTEX_ERRORCHECK);
	
	//pthread_mutex_init(&m1,NULL);

        //this initializes the mutex to unlocked state and sets the
        //attributes appropriately !!!
        //a mutex is a lock by nature - it does not have a counter
        // it has 2 states - unlocked state and locked state
        //if pthread_mutex_lock() is invoked and the lock state is
        //unlocked, lock state is changed to locked and pthread_mutex_lock()
        //returns !!!
        //pthread_mutex_lock() will block the current thread, if the 
        //lock state is locked - if another thread uses pthread_mutex_unlock(),
        //the blocked thread is woken up - before waking up the blocked thread,
        //the state of the mutex is set to unlocked state - one more subtle
        //aspect is that there is not increment operation 
        //if an unlocked mutex is again unlocked, operation is undefined
        //it depends on the implementation !!!  


	pthread_mutex_init(&m1,&ma1);


        //we are initializing condition variables with default 
        //attributes - you may change the attributes, if required -
        //refer to manual page !!!

        pthread_cond_init(&eq_cv1,NULL); //the condition variable
                                      //is initialized to def. attributes
	

        pthread_cond_init(&dq_cv2,NULL); //the condition variable


	pthread_attr_setschedpolicy(&tha1,SCHED_FIFO); //policy attribute
	p1.sched_priority = 10;
	pthread_attr_setschedparam(&tha1,&p1); //sched. param attrib- priority
	ret1 = pthread_create(&th_id1,&tha1,thread_func1,NULL);
	if(ret1)     
	{
		printf("error in thread creation\n");
		exit(1);
	}
	printf("thread created\n");

	policy1 = SCHED_FIFO;
	p1.sched_priority = 15;

	ret1 = pthread_setschedparam(pthread_self(),policy1,&p1); //changing priority of main thread

	ret1 = pthread_create(&th_id2,&tha1,thread_func2,NULL);
	if(ret1)
	{
		printf("error in thread creation\n");
		exit(1);
	}

	printf("thread2 created\n");

	pthread_join(th_id1,NULL);
	pthread_join(th_id2,NULL);
	
	exit(0);
}
