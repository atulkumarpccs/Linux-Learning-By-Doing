#include<pthread.h>
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<time.h>
#include<errno.h>
#include<unistd.h>
#include<fcntl.h>
#include<sys/types.h>

pthread_t th_id1,th_id2;
pthread_t th_id[3];

pthread_mutex_t m1;


char *fifo_list[] = { "/tmp/serv_fifo1", "/tmp/serv_fifo2",\
                       "/tmp/serv_fifo3"};


void cleanup1(void *arg1)
{

  printf("in the cancel handler1\n");

} 

void cleanup2(void *arg1)
{

  printf("in the cancel handler2\n");

  pthread_mutex_unlock(&m1);

}


void *thread_func1(void *arg)
{

  int policy1,ret1;
  struct sched_param p1;
  int st1,st2;

  struct timespec ts1;

  pthread_cleanup_push(cleanup1,NULL);
  pthread_cleanup_push(cleanup2,NULL);
  

  printf("I am in second thread and pid is %lu .. ppid is\
          %lu\n", getpid(),getppid());

  ret1 = pthread_getschedparam(pthread_self(),&policy1, &p1);

  if(!ret1) { 
     printf("the policy is %lu .. priority is %lu\n",\
             policy1,p1.sched_priority);

  }


  pthread_mutex_lock(&m1);


  ts1.tv_sec = 2;
  ts1.tv_nsec =0;

  nanosleep(&ts1,NULL);

  //while(1){

    printf("in new thread high\n");
    //sched_yield();
  //}

  pthread_mutex_unlock(&m1);

  pthread_cleanup_pop(0);
  pthread_cleanup_pop(0);

  pthread_exit(NULL);

} 

void *thread_func2(void *arg)
{

  int policy1,ret1;
  struct sched_param p1;
  

  printf("I am in third thread and pid is %lu .. ppid is\
          %lu\n", getpid(),getppid());

  ret1 = pthread_getschedparam(pthread_self(),&policy1, &p1);

  if(!ret1) { 
     printf("the policy is %lu .. priority is %lu\n",\
             policy1,p1.sched_priority);

  }
  while(1){

    printf("in new thread low\n");
    sched_yield();
  }

  pthread_exit(NULL);

}

void *thread_fifo(void *arg)
{

   int index,npfd,ret;
   char buf[512];

   index = (int) arg;


   npfd = open(fifo_list[index], O_RDONLY);

   //may terminate the entire process or a single thread
   if(npfd < 0) { perror("error in fifo creation"); exit(4); }

   while(1)
   {

      ret = read(npfd,buf,sizeof(buf));
      if(ret<0) { perror("error in reading fifo"); exit(5);} 
      if(ret>0) write(STDOUT_FILENO,buf,strlen(buf)+1);
   }

 pthread_exit(NULL);

}  
  

int main()
{


  int ret1,ret2;

  sigset_t s1,s2;

  pthread_attr_t tha1,tha2;


  pthread_mutexattr_t ma1;

  struct sched_param p1,p2;

  int policy1 , policy2;


  //alarm(3);


  printf("I am in main thread and pid is %lu .. ppid is\ 
          %lu\n", getpid(),getppid());


  sigfillset(&s1); // masking all signals in the process

  ret1 = mkfifo(fifo_list[0], 0600);  //creating a FIFO

  if(ret1 < 0 && errno != EEXIST){
        perror("error in mkfifo ..1"); exit(1);
  }
  
  ret1 = mkfifo(fifo_list[1], 0600);  //creating a FIFO
  if(ret1 < 0 && errno != EEXIST){
        perror("error in mkfifo ..2"); exit(1);
  }
  
  ret1 = mkfifo(fifo_list[2], 0600);  //creating a FIFO
  if(ret1 < 0 && errno != EEXIST){
        perror("error in mkfifo ..3"); exit(1);
  }

  pthread_attr_init(&tha1); 
 
  ret1 = pthread_create(&th_id[0],&tha1,thread_fifo,(void *)0);
  if(ret1){ printf("error in thread creation1\n"); exit(1);}
  printf("thread1 created\n"); 

  ret1 = pthread_create(&th_id[1],&tha1,thread_fifo,(void *)1);
  if(ret1){ printf("error in thread creation1\n"); exit(1);}
  printf("thread1 created\n"); 
  
  ret1 = pthread_create(&th_id[2],&tha1,thread_fifo,(void *)2);
  if(ret1){ printf("error in thread creation1\n"); exit(1);}
  printf("thread1 created\n"); 


  
  pthread_join(th_id[0],NULL);
  pthread_join(th_id[1],NULL);
  pthread_join(th_id[2],NULL);


  exit(0);
  

}
