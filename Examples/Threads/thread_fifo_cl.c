#include<pthread.h>
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<time.h>
#include<errno.h>
#include<unistd.h>
#include<fcntl.h>
#include<sys/types.h>

pthread_t th_id1,th_id2;
pthread_t th_id[3];

pthread_mutex_t m1;



int main(int argc, char *argv[])
{

   int index,npfd,ret;
   char buf[512];

    


   if(argc < 3) { printf("error in no. of arguments: pass fifoname and\
                  data string\n");
                  exit(1);
   }   

   npfd = open(argv[1], O_WRONLY);

   //may terminate the entire process or a single thread
   if(npfd < 0) { perror("error in fifo opening in client"); exit(2); }

   strcpy(buf,argv[2]);

   while(1)
   {

      ret = write(npfd,buf,strlen(buf)+1);
      if(ret<0) { perror("error in reading fifo"); exit(3);} 

      if(ret > 0) printf("no. of bytes written is %lu\n",ret);
   }

   exit(0);

}  
  

