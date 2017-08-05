/* 3.create a child process and compile any of programs in the child process.Parent process must use waitpid() to collect 
 * the termination status of the child process and print a message accordingly to the user. */
#include<stdio.h>
#include<stdlib.h>
#include<sys/types.h>
#include<sys/wait.h>
#include<unistd.h>
int main()
{
	int ret,status;
	ret=fork();
	if(ret==0)
	{
		printf("child process\n");
		ret=execl("/usr/bin/gcc","gcc","a1.c", "-o", "g1",NULL);
		if(ret<0)
		{
			perror("error in execl\n");
			exit(5);
		}
	}
	else
	{
		ret=waitpid(-1,&status,0);
		printf("parent process\n");
	}
	return 0;
}