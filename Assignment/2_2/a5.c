#include<stdio.h>
#include<ulimit.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include<stdlib.h>
#include<signal.h>
int main()
{
	sigset_t set1,set2;
	int ret,status,k=0;
	while(1)
	{
		k++;
		ret=fork();
		if(ret==-1)
		{
			printf("max no of childs created\n");	
			break;
			//exit(0);
		}
		if(ret==0)
		{				
			size_t nbytes =0x4000000;
			char *ptr = (char *) malloc(nbytes);
			//printf("size of file=%d\n",ulimit(UL_GETFSIZE,NEWLIMIT));//second parameter needed
			size_t i; 
			const size_t stride = sysconf(_SC_PAGE_SIZE); 
			for (i = 0; i < nbytes; i += stride) 
			{ 
				ptr[i] = 0; 
			} 
			if (ptr == NULL) 
		       	{ 
		          perror("malloc failure\n"); 
		          exit(1); 
	      		} 	
			exit(0);		
		}
		if(ret>0)
		{
				printf("creating child=%d\n",k);
		}
	}
	if(ret>0||ret==-1)
	{
		int count=0;
		while(1)
		{
			ret=waitpid(-1,&status,NULL);
			count++;
			if(ret==-1)
			{
				printf("no of childs=%d\n",count);
				printf("all pd freed\n");
				exit(0);
			}
		}
	}
	return 0;
}