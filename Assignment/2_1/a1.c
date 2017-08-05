#include<stdio.h>
#include<stdlib.h>
int main()
{
	int ret,status;
	unsigned long int i=0;
	while(1)
	{
		i++;
		ret=fork();
		if(ret>0)
		{
		 printf("%dst process\n",i);
		}
		else if(ret==-1)
		{
		perror("all pds allocated\n");
		break;
		}
		if(ret==0)
		{
			exit(0);
		}
	}
	if(ret>0 || ret==-1)
	{
		while(1)
		{
			ret=waitpid(-1,&status,0);
			if(ret<0)
			{
				printf("all pds deleted\n");
				break;
			}
		}
	}
	return 0;
}
