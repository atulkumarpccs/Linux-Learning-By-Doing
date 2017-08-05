#include<errno.h>
#include<asm/unistd.h>
#include<unistd.h>
#include<stdio.h>
#include<stdlib.h>
#include "syscallmacros.h"
#define __NR_test_syscall 327
extern char **environ;

_syscall3(ssize_t,write,int,fd,const void *,buf,size_t,count)

_syscall3(long,open,const char *,filename,int,flags,int,mode)

_syscall0(pid_t,getpid)

//_syscall1(long,test_syscall,int,val)
_syscall2(long,test_syscall,long*,pid,long*,tgid)


int main()
{
    

   int ret,ret1;
   long pid, tgid;

   char str[1024];

   ret = snprintf(str,sizeof(str), "the pid of the process is %d\n", getpid());
  
   ret = test_syscall(&pid,&tgid);  //testing our call

   if(ret<0){ perror("error in test_syscall"); exit(2);}

   printf("the test values are  %lu and %lu\n", (unsigned long)pid,(unsigned long)tgid) ;
   //printf("the test value is %u\n", (unsigned long)-129);
   //printf("the test value is %u\n", (unsigned long)-30);
   sleep(30); 
   exit(0); 
}