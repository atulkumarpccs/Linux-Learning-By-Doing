#include<stdio.h>
#include<stdlib.h>
#include<signal.h>
#include<unistd.h>
void sighandler(int signo)
{
	switch(signo)
	{
		case SIGKILL:
				printf("HANDLE KILL SIGNAL\n");
				break;
		case SIGINT:
				printf("HANDLE SIGINT SIGNAL\n");
				break;
		case SIGTERM:
				printf("HANDLE SIGTERM SIGNAL\n");
				break;
		case SIGQUIT:
				printf("HANDLE SIGQUIT SIGNAL\n");
				break;
		case SIGSTOP:
				printf("HANDLE SIGSTOP SIGNAL\n");
				break;
		case SIGTSTP:
				printf("HANDLE STP SIGNAL");
				break;
		case SIGALRM:
				printf("HANDLE STP SIGNAL");
				break;
		case SIGUSR1:
				printf("HANDLE STP SIGNAL");
				break;
		case SIGCHLD:
				printf("HANDLE CHILD SIGNAL");
				break;
		default:
				printf("NO SINGNAL CATCH");
	}
}
int main()
{
	int ret=1,i;
	struct sigaction act[9];
	sigset_t set1;
	sigfillset(&set1);
	sigdelset(&set1,SIGINT);
	sigdelset(&set1,SIGKILL);
	sigdelset(&set1,SIGTERM);
	sigdelset(&set1,SIGQUIT);
	sigdelset(&set1,SIGSTOP);
	sigdelset(&set1,SIGTSTP);
	sigdelset(&set1,SIGALRM);
	sigdelset(&set1,SIGUSR1);
	sigdelset(&set1,SIGCHLD);
	for(i=0;i<=5;i++)
	{
		act[i].sa_handler=sighandler;
		act[i].sa_flags=0;
		sigfillset(&act[i].sa_mask);
	}
	sigaction(SIGINT,&act[0],0);
	sigaction(SIGKILL,&act[1],0);
	sigaction(SIGTERM,&act[2],0);
	sigaction(SIGQUIT,&act[3],0);
	sigaction(SIGSTOP,&act[4],0);
	sigaction(SIGTSTP,&act[5],0);
	sigaction(SIGALRM,&act[5],0);
	sigaction(SIGUSR1,&act[5],0);
	sigaction(SIGCHLD,&act[5],0);
	/*ret=fork();
	if(ret==0)
	{
		printf("inside child\n");
		exit(0);
	}
	else*/ if(ret>0)
	{	
		sigsuspend(&set1);
		printf("inside parent\n");
	}
	exit(0);
}