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
  int st1,st2,sig_no;

  sigset_t s1,s2;

  struct timespec ts1;

  sigempty(&s1);

  sigaddset(&s1,SIGINT);
  sigaddset(&s1,SIGTERM);


  printf("I am in second thread and pid is %lu .. ppid is\
          %lu\n", getpid(),getppid());

  ret1 = pthread_getschedparam(pthread_self(),&policy1, &p1);

  if(!ret1) { 
     printf("the policy is %lu .. priority is %lu\n",\
             policy1,p1.sched_priority);

  }
 //another way to handle signals in threads is using 
 //sigwait() system call - this is very different from
 //the earlier signal handling/action using sigaction()
 //1.when sigwait is called, the thread blocks - waiting for 
 //  signals
 //2.it will wake-up when one or more signals are pending
 //  for this thread and the signals are set in the bit-mask
 //  that is passed to sigwait() as the first parameter  
 //3.when a signal of the type described in (2) occurs,
 //  the thread will be woken-up,the signal no. will be
 //  returned in the sig_no parameter passed to sigwait()
 //  as second parameter and the sigwait() system call
 //  resumes the thread without executing any signal handler
 //4.one more constain is that no signal handler must be
 //  installed for signals set in the first parameter passed
 //  to sigwait() 

  while(1) { 
      //all signals set in s1 must be masked before we enter 
      sigwait(&s1,&sig_no);  //

      if(signo == SIGINT) { } //here we handle the signal
      if(signo == SIGTERM) { } //here we handle another signal
     
      // do whatever work needs to be done here !!!
      // 


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
  sigdelset(&s1,SIGINT);
  sigdelset(&s1,SIGTERM); 
  //sigprocmask(SIG_SETMASK,&s1,NULL); //really - setting the signal mask
  sigprocmask(SIG_SETMASK,&s1,NULL); 

  //use sigaction to install handlers


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
