#include<stdio.h>
#include<stdlib.h>
#include<sys/sem.h>
#include<sys/shm.h>
#include<unistd.h>
#include<sys/types.h>
#define KEY 1
union u
{
	int data;
	unsigned short *array;
};
int main()
{
	union u du;
	int semob;
	int stat;
	struct sembuf sb;
	int ret1,ret;
	du.data = 0;
	semob=semget(KEY,1,IPC_CREAT|0600);
        semctl(semob,0,SETVAL,du);
	ret = fork();
	if(ret>0)
	{
		printf("enter parent\n");
		ret1 = semctl(semob , 0 , GETVAL);
		printf("\np:sem before : %d\n",ret1); 
		sb.sem_num=0;
		sb.sem_op=+1;
		sb.sem_flg=0;
		semop(semob,&sb,1);
		ret1 = semctl(semob , 0 , GETVAL);
		printf("\np:sem after : %d\n",ret1);
		printf("exit parent\n");
        }		
	else if(ret==0)
	{
		printf("enter child\n");
		ret1 = semctl(semob , 0 , GETVAL);
		printf("\nc:sem before : %d\n",ret1);
		sb.sem_num=0;
		sb.sem_op=-1;
		sb.sem_flg=0;
		semop(semob,&sb,1);
		ret1 = semctl(semob , 0 , GETVAL);
		printf("\nc:sem after : %d\n",ret1);
		printf("exit child\n");
	}
	exit(0);
}