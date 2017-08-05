#include<unistd.h>
#include<linux/fcntl.h>
#include<sys/types.h>
#include "syscallmacros.h"
#include <errno.h>
_syscall3(ssize_t,read,int,fd,const void *,buf,size_t,count)

int main()
{

   int fd,ret,buf[1024];
   fd = open("/proc/readme",O_RDONLY);
   if(fd<0){
           perror("error in opening");
           exit(1);
   }
   //lseek(fd,22,SEEK_SET);

   ret = read(fd,buf,11);
   
   printf("the no. characters returned is %d\n", ret);

   if(ret>0) { write(STDOUT_FILENO,buf,ret); }
   
   //ret = read(fd,buf,11);
   ret = read(fd,buf,11);

   //ret = read(fd,buf,22);
   printf("the no. characters returned is %d\n", ret);

   if(ret>0) { write(STDOUT_FILENO,buf,ret); }
   //pause();
   exit(0);

}
