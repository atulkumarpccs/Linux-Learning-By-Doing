#include<stdio.h>
#include<stdlib.h>
#include<sys/types.h>
#include<sys/wait.h>
#include<unistd.h>
int main(int argc,char *argv[])
{
	int ret,status,i=0;
	while(i++<5)
	{
		ret=fork();
		if(ret==0)
		{
			printf("child process\n");
			ret=execl("/usr/bin/gcc","gcc",argv[i],"-c",NULL);
			if(ret<0)
			{
				perror("error in execl\n");
				exit(1);
			 }
			exit(0);
		}
		else
		{
			if(ret==-1)
			{
				perror("max child created\n");
				break;
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
					ret=execl("/usr/bin/gcc","gcc","a.o","b.o","d.o","e.o","f.o","-o","result",NULL);
					if(ret<0)
					{
						perror("error in execl\n");
						exit(1);
					}
				}
		}
	}
	return 0;
}
