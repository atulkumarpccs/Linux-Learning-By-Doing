//
#define _GNU_SOURCE
#include <unistd.h>
#include <sys/syscall.h>
#include <sys/types.h>

#include <errno.h>
#include <pthread.h>
#include <semaphore.h>
#include <signal.h>
#include "buffer.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include<sched.h>
#include<sys/time.h>
#include<sys/resource.h>
#include<sys/mman.h>

//
//
static buffer_t buffer[5][BUFSIZE];
static buffer_t user_input_buffer[1024];

//remove this approach for initialization of mutexes and use
//the approach given in thread_mutex_cond.c 

static pthread_mutex_t  input_buffer_lock   = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t  data_buffer_lock    = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t  display_buffer_lock = PTHREAD_MUTEX_INITIALIZER;
static int bufin = 0;
static int bufout = 0;
static int freeslots = 0;
static int filledslots = 0;
static volatile sig_atomic_t initdone = 0;
static int initerror = 0;
static pthread_once_t initonce = PTHREAD_ONCE_INIT;

//these 

static sem_t semitems;   //these are semaphore objects used for lightweight semaphores
static sem_t semslots;
static sem_t seminput;

//
//remove this initialization approach for semaphores and 
//and add the initialization for semaphores in the 
//main thread of this process, before creating new,additional threads
//
//
static int bufferinit(void) { /* called exactly once by getitem and putitem  */
   int error;
   if (sem_init(&semitems, 0, 0))  //sem_init() is used to initialize a semaphore
      return errno;
   if (sem_init(&semslots, 0, 5)) {
      error = errno;
      sem_destroy(&semitems);
   }                                      /* free the other semaphore */
   if (sem_init(&seminput, 0, 0)) {
      error = errno;
      sem_destroy(&semslots);      return error;
      sem_destroy(&semitems);      return error;
   }
   return 0;
}

//
//
static void initialization(void) {
   initerror = bufferinit();
   if (!initerror)
      initdone = 1;
}

//
//
static int bufferinitonce(void) {          /* initialize buffer at most once */
   int error;
   if ((error = pthread_once(&initonce, initialization)) )
      return error;
   return initerror;
}

//
//
int getitem(buffer_t *item) {  /* remove item from buffer and put in *itemp */
   int error;
   if (!initdone)
      bufferinitonce();
   while (((error = sem_wait(&semitems)) == -1) && (errno == EINTR)) ;
   if (error)
      return errno;
   if ((error = pthread_mutex_lock(&data_buffer_lock)))
      return error; 
   strncpy(item,buffer[bufout],1024);  //copy string
   //*itemp = buffer[bufout];
   bufout = (bufout + 1) % 5;

   filledslots--;
   freeslots++;  

   printf("2..get item \n");
   if ((error = pthread_mutex_unlock(&data_buffer_lock)))
      return error;
   if (sem_post(&semslots) == -1)  
      return errno; 
   return 0; 
}

//this function is used in the producer thread 
//all basic principles are the same !!!
//just the mechanisms are different !!!
//
//
int putitem(buffer_t *item) {                    /* insert item in the buffer */
   int error;
   if (!initdone)
      bufferinitonce();
   while (((error = sem_wait(&semslots)) == -1) && (errno == EINTR)) ;
   if (error)
      return errno;
   if ((error = pthread_mutex_lock(&data_buffer_lock)))
      return error;    
   //buffer[bufin] = item;

   strncpy(buffer[bufin],item,1024);  //copy string
   bufin = (bufin + 1) % 5;
   
   filledslots++;
   freeslots--; 
   printf("1..put item \n");
   if ((error = pthread_mutex_unlock(&data_buffer_lock)))
      return error; 
   if (sem_post(&semitems) == -1)  
      return errno; 
   return 0; 
}


//in this example, threads are not just for performance 
//their objectives is to block discreetly and wake up discreetly !!!


//
//
void *user_input_thread(void *arg)
{
  
   int ret;

   char pdatabuf[1024];

   while(1){

     printf("please input a maximum of 1023 character string - :)");
     fflush(stdout);
     
     ret = fgets(pdatabuf,sizeof(pdatabuf),stdin);    //produce
     if(ret == 0) exit(3); //fatal error
 
     //printf("data input is %s\n", pdatabuf);
 
     pthread_mutex_lock(&input_buffer_lock);

     strncpy(user_input_buffer,pdatabuf,sizeof(user_input_buffer));

     pthread_mutex_unlock(&input_buffer_lock);

     //printf("data input is %s\n", user_input_buffer);
     sem_post(&seminput);
    
     //sched_yield();


   }   

pthread_exit(NULL); //you should never reach here !!!

}


void *producer_thread(void *arg)
{
  
   int ret;
   char pdatabuf[1024];


   while(1){

     //fgets(pdatabuf, sizeof(pdatabuf), stdin);                 //produce
     //another form of synchronization 
     //in these cases, semaphores are only used for 
     //synchronization, not locks !!!

     sem_wait(&seminput);     

     pthread_mutex_lock(&input_buffer_lock);

     ret = putitem(user_input_buffer);

     pthread_mutex_unlock(&input_buffer_lock);

     if(ret != 0) exit(3); //fatal error

   }   

pthread_exit(NULL);

} 

//
//
void *consumer_thread(void *arg)
{
    
   int ret;
   char cdatabuf[1024];

   while(1){

        ret = getitem(cdatabuf);                             //consume
        if(ret != 0) exit(4); //fatal error
        //printf("the received data is %s\n", cdatabuf);
   }

pthread_exit(NULL);

} 

void *display_status_thread(void *arg)
{
    
   int ret;
   char cdatabuf[1024];

   sigset_t set1;

   sigemptyset(&set1);
   sigaddset(&set1,SIGINT);
   sigaddset(&set1,SIGTERM);

   setpriority(PRIO_PROCESS, syscall(SYS_gettid) , +19); 

   pthread_sigmask(SIG_UNBLOCK,&set1,NULL);

   while(1){

        pthread_mutex_lock(&data_buffer_lock);

        printf("the free slots is %d \n and filled slots is  %d\n",\
                freeslots,filledslots );

        pthread_mutex_unlock(&data_buffer_lock);
   }

pthread_exit(NULL);

}


//
//
pthread_t thid1,thid2,thid3,thid4;
int main()
{

   int ret;
   void *ptr = NULL;
   sigset_t set1,set2;

   struct sched_param param1;

   pthread_attr_t pthread_attr1,pthread_attr4;

   sigfillset(&set1);

   sigprocmask(SIG_BLOCK,&set1,&set2);

   //setpriority(PRIO_PROCESS,getpid(), +19); 


   //install all the signal handlers here-before any thread is 
   //created
  //ptr = malloc(32*1024);

     //try to understand mmap()'s working with the help of man page
     //???

     //mmap allocates memory anonymously - this is not same 
     //as heap - it is just dynamic memory in the process !!!
     //do not get confused - conventional heap is typically 
     //use by dynamic memory management library!!!
     
     //mmap() may be used in this form or subtly in a
     //different form in real time !!! 

     ptr = mmap(NULL,getpagesize()*8, PROT_WRITE, \
                MAP_PRIVATE|MAP_ANONYMOUS, -1 , 0);

   if(ptr!=NULL){

      printf("setting the stack attributes of a thread\n"); 
      
      //initialization of the thread attribute obj.
      ret = pthread_attr_init(&pthread_attr4); 
      if(ret>0) { 
              printf("error in the thread attr initialization\n");  
      }
      //initialization of the stack attributes in the thread attribute obj.
      ret = pthread_attr_setstack(&pthread_attr4,ptr,32*1024);
      if(ret>0) {

              printf("error in the stack attribute settings\n");
      }

   }

  

   ret = pthread_create(&thid4,&pthread_attr4,display_status_thread,NULL);

   if(ret>0) { printf("error in thread creation for producer\n"); exit(4); }   
   pthread_attr_init(&pthread_attr1); 
   pthread_attr_setschedpolicy(&pthread_attr1,SCHED_FIFO);

   param1.sched_priority = 1;
   pthread_attr_setschedparam(&pthread_attr1, &param1);  
  
   //this is an implementation peculiarity !!
   //even if you set the scheduling paramters for a thread, 
   //it will inherit the scheduling parameters of the creating 
   //thread unless we set the below mentioned special attribute !!!
 
   pthread_attr_setinheritsched(&pthread_attr1,PTHREAD_EXPLICIT_SCHED); 


   ret = pthread_create(&thid1,&pthread_attr1,user_input_thread,NULL);
   if(ret>0) { printf("error in thread creation for consumer\n"); exit(1); }

   ret = pthread_create(&thid2,NULL,consumer_thread,NULL);
   if(ret>0) { printf("error in thread creation for consumer\n"); exit(2); } 

   ret = pthread_create(&thid3,NULL,producer_thread,NULL);
   if(ret>0) { printf("error in thread creation for producer\n"); exit(3); } 


   pthread_join(thid1,NULL);

   pthread_join(thid2,NULL);

   pthread_join(thid3,NULL);
   
   pthread_join(thid4,NULL);

   exit(0);

}






