#include<stdio.h>
#include<stdlib.h>
#include<signal.h>
void sichld(int signo)
{ 
	int status;
  	int ret=waitpid(-1,&status,0);
  	if(ret>0)
  		printf("child pd teminated\n");
}
int main()
{
	sigset_t sig1;
	struct sigaction act1;
	int ret,status,i=0;
	act1.sa_handler=sichld;
	act1.sa_flags=0;
	sigfillset(&act1.sa_mask);
	sigaction(SIGCHLD,&act1,0);
	while(i++<5)
	{
		ret=fork();
		if(ret<0)
		{
			perror("no pd available\n");
			exit(0);
		}
		if(ret>0)
		{
			//printf("in parent context1\n");
		} 
		if(ret==0)
		{
			sched_yield();
			printf("%dgoing to child\n",i);
			exit(0);
		}
	}
	if(ret>0)
	{
		sigset_t sig1;
		i=0;
		sigfillset(&sig1);
		sigdelset(&sig1,SIGCHLD);
		while(i++<5)
		{
			sigsuspend(&sig1);
			printf("in parent context2\n");
		}
	}
	exit(0);
}