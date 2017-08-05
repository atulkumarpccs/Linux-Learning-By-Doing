/* 2. Create 5 processes but not from the common parent. Meaning, each child creates a new process.
      clean­up the children using  waitpid() */
#include<stdio.h>
#include<stdlib.h>
#include<sys/types.h>
#include<sys/wait.h>
#include<unistd.h>
int main()
{
	int ret,status,i=0;
	label:
		ret=fork();
		if(ret==0)
		{
			printf("child%d==%d==%d\n",i+1,getpid(),getppid());
			i++;
			if(i<=5)
			{
				printf("%d\n",i);
				goto label;
			}
		}
		else
		{
			waitpid(-1,&status,0);
			printf("\n parent process\n");
		}
	
	return 0;
}
