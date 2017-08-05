#include<unistd.h>
#include<sys/types.h>
#include<linux/fcntl.h>


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

   ret = read(fd,buf,11);
   printf("the no. characters returned is %d\n", ret);

   if(ret>0) { write(STDOUT_FILENO,buf,ret); }
   

   while(1){

   ret = read(fd,buf,11);

   if(ret==0) break ;
   
   printf("the no. characters returned is %d\n", ret);

   if(ret>0) { write(STDOUT_FILENO,buf,ret); } 

   }

   
   //ret = read(fd,buf,11);
   //ret = read(fd,buf,11);

   //ret = read(fd,buf,22);
   //printf("the no. characters returned is %d\n", ret);

   //if(ret>0) { write(STDOUT_FILENO,buf,ret); }
   //pause();
   exit(0);

}
