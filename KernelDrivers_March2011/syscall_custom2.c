#define _GNU_SOURCE
#include<unistd.h>
#include<stdio.h>
#include<stdlib.h>
#include<sys/syscall.h>
#include<sys/types.h>

#define SYS_test_syscall 328 
int main()
{

   int ret,ret1;

   char str[1024];

   ret = snprintf(str,sizeof(str), "the pid of the process is %d\n", getpid());
  
   ret1 = syscall(SYS_test_syscall,777);

   if(ret1<0){ perror("error in write syscall"); exit(1);}

   exit(0); 

}
