#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#define SIZE 128	// 共享内存大小
char *MUTEX_NAME = "mutex";		// 自定义有名信号量
char *SYNC_NAME = "sync";		// 同步信号量

int main()
{
	key_t key;		//标志共享内存的自定义键值
	key = 2018;

	sem_t *mutex;		// 互斥信号量
	sem_t *sync;		// 同步信号量

	// open mutex
	mutex = sem_open(MUTEX_NAME, O_CREAT);
	if(mutex == SEM_FAILED)
	{
		printf("Receiver open mutex failed!\n");
		exit(0);
	}

	// open sync
	sync = sem_open(SYNC_NAME, O_CREAT);
	if(sync == SEM_FAILED)
	{
		printf("Receiver open sync failed!\n");
		exit(0);
	}


	// Open shared memeory
	int shmid;
	shmid = shmget(key, SIZE, 0666);
	if(shmid < 0)
	{
		printf("Receiver open share memory failed!\n");
		exit(0);
	}

	// shared memory map to program address space
	char *shmaddr;
	shmaddr = shmat(shmid, NULL, 0);

	char *message;
	// Receiver receive content from Sender
	sem_wait(mutex);
	strcpy(message, shmaddr);
	sem_post(mutex);
	printf("Receiver received content: %s\n", message);

	// Receiver send over
	char *reply = "over";
	sem_wait(mutex);
	strcpy(shmaddr, reply);
	sem_post(mutex);

	// Receiver release token
	sem_post(sync);
	sem_close(mutex);
	sem_close(sync);

	printf("Receiver process end!\n");

	return 0;
}
