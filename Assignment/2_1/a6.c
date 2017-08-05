#include<stdio.h>
#include<stdlib.h>
#include<signal.h>
#include<unistd.h>
int main()
{
	printf("parent=%d,process=%d\n",getppid(),getpid());
	int ret=kill(getppid(),SIGKILL);//cannt use 1
	if(ret==0)
		printf("parent killed\n");
	else if(ret==-1)
		printf("kill() failed\n");
	printf("parent=%d,process=%d",getppid(),getpid());
	exit(0);
}

