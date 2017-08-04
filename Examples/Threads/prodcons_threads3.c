//
//
#include <errno.h>
#include <pthread.h>
#include <semaphore.h>
#include <signal.h>
#include "buffer.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

//
//
static buffer_t buffer[5][BUFSIZE];
static buffer_t user_input_buffer[1024];
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
static sem_t semitems;
static sem_t semslots;
static sem_t seminput;

//
//
static int bufferinit(void) { /* called exactly once by getitem and putitem  */
   int error;
   if (sem_init(&semitems, 0, 0))
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


//
//
void *user_input_thread(void *arg)
{
  
   int ret;

   char pdatabuf[1024];


   //pthread_exit(NULL); 
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
     sem_post(&seminput);//V()
    
     //sched_yield();


   }//end of while loop   

pthread_exit(NULL);

}


void *producer_thread(void *arg)
{
  
   int ret;
   char pdatabuf[1024];


   while(1){

     //fgets(pdatabuf, sizeof(pdatabuf), stdin);                 //produce

     sem_wait(&seminput);    //P() 

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

   pthread_sigmask(SIG_UNBLOCK,&set1,NULL);

   while(1){

        pthread_mutex_lock(&data_buffer_lock);

        //printf("the free slots is %d \n and filled slots is  %d\n",\
                freeslots,filledslots );
        pthread_testcancel(); //whenever this api is called, 
                              //cancellation request is checked
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

   sigset_t set1,set2;


   sigfillset(&set1);

   sigprocmask(SIG_BLOCK,&set1,&set2);

   //install all the signal handlers here-before any thread is 
   //created
  


   //ret = pthread_create(&thid4,NULL,display_status_thread,NULL);
   //if(ret>0) { printf("error in thread creation for producer\n"); exit(4); }   
   

   //ret = pthread_create(&thid1,NULL,user_input_thread,NULL);
   //if(ret>0) { printf("error in thread creation for consumer\n"); exit(1); }

  //ret = pthread_create(&thid2,NULL,consumer_thread,NULL);
  // if(ret>0) { printf("error in thread creation for consumer\n"); exit(2); } 

 //ret = pthread_create(&thid3,NULL,producer_thread,NULL);
  // if(ret>0) { printf("error in thread creation for producer\n"); exit(3); } 
   //pthread_exit(NULL);

   //pthread_join(thid1,NULL);  //current thread will block until 
                              //thread with id thid1 terminates 

   //pthread_join(thid2,NULL);

   //pthread_join(thid3,NULL);
   
   //pthread_join(thid4,NULL);

   pause();

   exit(0);

}