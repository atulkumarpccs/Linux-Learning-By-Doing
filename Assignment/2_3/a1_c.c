#include<stdio.h>
#include<stdlib.h>
#include<sys/types.h>
#include<unistd.h>
#include<sys/shm.h>
#include<sys/sem.h>
#define KEY 111
struct shmem
{
	char a[50];
	int count,size;
	int in,out;
};
union semun
{
	int data;
};
int main()
{
	union semun u1;
	int shmid1,semid1,ret,value;
	struct sembuf sb1;
	struct shmem *shma;	
	shmid1=shmget(KEY,sizeof(struct shmem),IPC_CREAT|0600);
     	if(shmid1<0)
		{
		  perror("cannt allocate VAD\n");
		  exit(0);
		}	
	semid1=semget(KEY,1,IPC_CREAT|0600);
	if(semid1<0)
		{
		perror("cannt allocate semaphore object\n"); 
		exit(0);
		}	       
	
	shma=shmat(shmid1,0,0);	
        if(shma<0)
		{  
		perror("cannt attach process to shared memory\n");
		exit(0);
		}
	while(1)
	{
		sb1.sem_num=1;
		sb1.sem_op=-1;
		sb1.sem_flg=0;
		semop(semid1,&sb1,1);
		if(shma->count==0)
			continue;
		else
                {
			printf("%c",shma->a[shma->in]);
			shma->in=(shma->in+1)% (shma->size);
			shma->count-=1;
		}
		sb1.sem_num=1;
		sb1.sem_op=+1;
		sb1.sem_flg=0;
		semop(semid1,&sb1,1);
	}
	exit(0);
}