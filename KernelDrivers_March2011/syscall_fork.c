extern char **environ;

#define __NR_test_syscall1 327

_syscall3(ssize_t,write,int,fd,const void *,buf,size_t,count)

_syscall3(long,open,const char *,filename,int,flags,int,mode)

_syscall0(pid_t,getpid)
_syscall0(pid_t,fork)

_syscall2(long,test_syscall1,unsigned int*,pid,unsigned int *,tgid)
//_syscall1(long,test_syscall2,struct syscall *, syscall2)
int main()
{
    

   int ret,ret1;
   unsigned int pid,tgid;

   char str[1024];



/*   ret = snprintf(str,sizeof(str), "the pid of the process is %d\n", getpid());
  
   ret1 = write(STDOUT_FILENO,str,ret+1);

   if(ret1<0){ perror("error in write syscall"); exit(1);}

   ret1 = getpid();
   

   if(ret1<0){ perror("error in open syscall"); exit(2);}*/


   ret1 = test_syscall1(&pid,&tgid);
   if(ret1<0) { perror("error in test_syscall"); exit(1); }

   printf("test_syscall returned pid %d and tgid %d\n",pid,tgid);


   exit(0); 
}
              


