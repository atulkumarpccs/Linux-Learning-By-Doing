#include<stdio.h>
#include<stdlib.h>
#include<sys/types.h>
#include<sys/wait.h>
#include<unistd.h>
int main()
{
	int i=1,ret,status;
	while(i++<=5)
	{
		ret=fork();
		if(ret==0)
		{
			printf("Inside child\n");
			exit(0);
		}
		else
		{
			if(ret>0)
			{
				printf("creating child\n");
				
			}
			else if(ret==-1)
			{
				perror("maximum childs created\n");
				exit(2);
			}
		}
	}
	if(ret>0)
	{
		while(1)
		{
			ret=waitpid(-1,&status,0);
			if(ret<0)
			{
				printf("all TASK_ZOMBIE pd released\n");
				break;
			}
			else
				printf("1 TASK_ZOMBIE pd released\n");
		}
	}
	exit(0);
}
