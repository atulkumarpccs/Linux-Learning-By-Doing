#include<stdio.h>
#include<stdlib.h>
#include<sys/ioctl.h>
#include<unistd.h>
int main()
{
   
   int ret=0;
   char bytes[2] = { 11,0 };

   bytes[1] = 1; //change this to whatever console of choice 

   ret = ioctl(STDIN_FILENO, TIOCLINUX, bytes);
   if(ret<0) { perror("error in ioctl"); exit(1); }

   printf("the console set for kernel diagnostic messages is %d\n", bytes[1]); 

   exit(0); 

} 