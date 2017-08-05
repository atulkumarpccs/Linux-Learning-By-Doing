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
	semid1=semget(KEY,1,IPC_CREAT|0600);
	shma=shmat(shmid1,0,0);	
        int r=fork();
	if(r>0)
	{


		sched_yield();
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
				printf("%c\n",shma->a[shma->in]);
				shma->in=(shma->in+1)% (shma->size);
				shma->count-=1;
			}
			sb1.sem_num=1;
			sb1.sem_op=+1;
			sb1.sem_flg=0;
			semop(semid1,&sb1,1);

		}
	}
	if(r==0)
	{
		shmid1=shmget(KEY,sizeof(struct shmem),IPC_CREAT|0600);
		semid1=semget(KEY,1,IPC_CREAT|0600);
		u1.data=1;
		semctl(semid1,0,SETALL,u1);		
		shma = shmat(shmid1,0,0);
		shma->in=0;
		shma->out=0;
		shma->count=0;
		shma->size=50;
		while(1)				 
		{
			sb1.sem_num=1;
			sb1.sem_op=-1;
			sb1.sem_flg=0;
			semop(semid1,&sb1,1);
			if(shma->in==shma->out && shma->count==shma->size)
				continue; 
			else
			{	
				scanf("%c",&ret);
				shma->a[shma->out]=ret;
				shma->out=((shma->out)+1)% (shma->size);
				shma->count+=1;
				
			}
			sb1.sem_num=1;
			sb1.sem_op=+1;
			sb1.sem_flg=0;
			semop(semid1,&sb1,1);
		}
	}	
	exit(0);
}