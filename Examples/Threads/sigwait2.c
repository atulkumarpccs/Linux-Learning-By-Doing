/*    repeat the above problem(2) without installing signal handlers; instead, use 
    sigwait() in the respective threads;  what is the advantage of this approach 
    over the previous approach ?  */

#include<stdio.h>
#include<pthread.h>
#include<errno.h>
#include<unistd.h>
#include<sys/types.h>
#include<fcntl.h>
#include<signal.h>
#include<stdlib.h>
#include<string.h>

/*void sig_int(int sig)
{
	printf("\n SIGINT \n");
	exit(0);
}

void sig_term(int sig)
{
	printf("\n SIGTERM \n");
	exit(0);
}

void sig_pipe(int sig)
{
	printf("\n SIGPIPE \n");
	exit(0);
}*/

pthread_t th_id[10];

void thread(int arg)
{
	printf("\n i am in thread %d \n",arg );

	//pthread_exit(NULL);
}

void thread_fun1(void *arg)
{
	sigset_t s1;		//bitmap 62 signals
	struct sigaction act1;
	int sig;

	sigemptyset(&s1);		//sets every signal bit in s1 
	sigaddset(&s1,SIGINT);  
	/*act1.sa_flags = 0;
	act1.sa_handler = sig_int;
	sigaction(SIGINT,&act1,NULL); */

	printf("I am in thread_func1\n");
	while(1)
	{
		sigwait(&s1, &sig);
		if(sig == SIGINT)
		{
			printf(" SIGINT\n");
			//exit(1);
		}
	}


	//pthread_exit(NULL);
}

void thread_fun2(void *arg )
{
	sigset_t s1;		//bitmap 62 signals
	struct sigaction act1;
        int signo;
	//sigfillset(&s1);		//sets every signal bit in s1 
	//sigdelset(&s1,SIGTERM);
	//pthread_sigmask(SIG_SETMASK,&s1,NULL);		//sigprocmask of thread
	
	/*act1.sa_flags = 0;
	act1.sa_handler = sig_term;
	sigaction(SIGTERM,&act1,NULL); */

	sigemptyset(&s1);		//sets every signal bit in s1 
	sigaddset(&s1,SIGTERM); 	sigwait(&s1, &signo);

	printf("I am in thread_func2\n");

	//pthread_exit(NULL);
}

void thread_fun3(void *arg)
{
	sigset_t s1;		//bitmap 62 signals
	struct sigaction act1;
        int signo;

//	sigfillset(&s1);		//sets every signal bit in s1 
//	sigdelset(&s1,SIGPIPE);
//	pthread_sigmask(SIG_SETMASK,&s1,NULL);		//sigprocmask of thread
	
	/*act1.sa_flags = 0;
	act1.sa_handler = sig_pipe;
	sigaction(SIGPIPE,&act1,NULL); */

        sigemptyset(&s1);		//sets every signal bit in s1 
	sigaddset(&s1,SIGPIPE); sigwait(&s1,&signo) ;

	printf("I am in thread_func3\n");

	//pthread_exit(NULL);
}


int main()
{
	int i,ret,pfd;
	pthread_attr_t th_a;
        sigset_t s1;
	
	printf("I am in main thread and pid is %lu .. ppid is %lu\n", getpid(),getppid());

        sigfillset(&s1);
        sigprocmask(SIG_SETMASK,&s1,NULL);


	pthread_attr_init(&th_a);
	
	//1st thread
	ret=pthread_create(&th_id[0],&th_a,thread_fun1,NULL);
	if(ret!=0)
	{
		perror("error in creating thread:");
		exit(1);
	}
	printf("created thread1 \n");

	//2nd thread
	ret=pthread_create(&th_id[1],&th_a,thread_fun2,NULL);
	if(ret!=0)
	{
		perror("error in creating thread:");
		exit(1);
	}
	printf("created thread2 \n");

	//3rd thread
	ret=pthread_create(&th_id[2],&th_a,thread_fun3,NULL);
	if(ret!=0)
	{
		perror("error in creating thread:");
		exit(1);
	}
	printf("created thread3 \n");
	
	//for loop for threads from 4th to 10th	
	for(i=3;i<10;i++)
	{
		ret=pthread_create(&th_id[i],&th_a,thread,i);
		if(ret!=0)
		{
			perror("error in creating thread:");
			exit(1);
		}
		
		printf("created thread%d\n",i);
	}

	for(i=0;i<10;i++)		//thread termination
	{
		printf("termination of %d thread \n", i);
		pthread_join(th_id[i],NULL);
	}

	while(1);

	exit(0);
}
