#include<errno.h>
#include<asm/unistd.h>
#include<unistd.h>
#include<stdio.h>
#include<stdlib.h>
#include "syscallmacros.h"

extern char **environ;

_syscall3(ssize_t,write,int,fd,const void *,buf,size_t,count)

_syscall3(long,open,const char *,filename,int,flags,int,mode)

_syscall0(pid_t,getpid)


int main()
{
    

   int ret,ret1;

   char str[1024];

   ret = snprintf(str,sizeof(str), "the pid of the process is %d\n", getpid());
  
   ret1 = write(STDOUT_FILENO,str,ret+1);

   if(ret1<0){ perror("error in write syscall"); exit(1);}

   ret1 = getpid();
   

   if(ret1<0){ perror("error in open syscall"); exit(2);}

   exit(0); 
}