#include<pthread.h>
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<time.h>
#include<errno.h>
#include<unistd.h>
#include<fcntl.h>
#include<sys/types.h>
#include<signal.h>



pthread_t th_id1,th_id2;
pthread_t th_id[3];

pthread_mutex_t m1;


char *fifo_list[] = { "/tmp/serv_fifo1", "/tmp/serv_fifo2",\
                       "/tmp/serv_fifo3"};

//you can write your signal handlers here
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


  printf("I am in second thread and pid is %lu .. ppid is\
          %lu\n", getpid(),getppid());

  ret1 = pthread_getschedparam(pthread_self(),&policy1, &p1);

  if(!ret1) { 
     printf("the policy is %lu .. priority is %lu\n",\
             policy1,p1.sched_priority);

  }

  while(1) { 
    printf("I am in thread_func1\n");
  }

  pthread_exit(NULL);

} 

void *thread_func3(void *arg)
{

  int policy1,ret1;
  struct sched_param p1;
  int st1,st2;

  //the signal mask field will be set the bit-mask value
  //of the signal mask field of the sibling thread that
  //created this thread
  sigset_t s1,s2;

  struct timespec ts1;

  sigfillset(&s1);

  sigdelset(&s1,SIGINT);

  pthread_sigmask(SIG_SETMASK,&s1,0); //handle SIGINT only


  printf("I am in second thread and pid is %lu .. ppid is\
          %lu\n", getpid(),getppid());

  ret1 = pthread_getschedparam(pthread_self(),&policy1, &p1);

  if(!ret1) { 
     printf("the policy is %lu .. priority is %lu\n",\
             policy1,p1.sched_priority);

  }

  while(1) { 
    printf("I am in thread_func3\n");
  }

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

  sigprocmask(SIG_SETMASK,&s1,NULL); //really - setting the signal mask

  //do not use signal() system call - obselete
  //use sigaction to install handlers
  //use sigaction() system call in order to install the 
  //signal action for each signal, as required

  pthread_attr_init(&tha1); 
 
  ret1 = pthread_create(&th_id[0],&tha1,thread_func1,(void *)0);
  if(ret1){ printf("error in thread creation1\n"); exit(1);}
  printf("thread1 created\n"); 
  

  ret1 = pthread_create(&th_id[1],&tha1,thread_func3,(void *)0);
  if(ret1){ printf("error in thread creation2\n"); exit(1);}
  printf("thread2 created\n"); 






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
