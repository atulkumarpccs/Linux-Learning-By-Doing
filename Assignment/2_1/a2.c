#include<stdio.h>
#include<stdlib.h>
int main()
{
	int status,ret;
	ret=fork();
	if(ret==0)
	{
		sched_yield();//yields processor
		printf("inside child\n");
		exit(0);
	}
	else
	{
		if(ret==-1)
		{
			perror("cannt allocate pd\n");
			exit(0);
		}
		if(ret>0)
		{
			printf("inside parent\n");
		}
	}
	return 0;
}
