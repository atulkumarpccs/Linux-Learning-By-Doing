#define _GNU_SOURCE
#include<unistd.h>
#include<stdio.h>
#include<stdlib.h>
#include<sys/syscall.h>
#include<sys/types.h>


int main()
{

   int ret,ret1;

   char str[1024];

   ret = snprintf(str,sizeof(str), "the pid of the process is %d\n", getpid());
  
   ret1 = syscall(SYS_write,STDOUT_FILENO,str,ret+1);

   if(ret1<0){ perror("error in write syscall"); exit(1);}

   ret1 = syscall(SYS_getpid);
   

   if(ret1<0){ perror("error in open syscall"); exit(2);}

   exit(0); 

}

              


