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
	mutex = sem_open(MUTEX_NAME, O_CREAT, 0644, 1);
	if(mutex == SEM_FAILED)
	{
		printf("Sender create mutex failed!\n");
		exit(0);
	}

	// open sync
	sync = sem_open(SYNC_NAME, O_CREAT, 0644, 0);
	if(sync == SEM_FAILED)
	{
		printf("Sender create sync failed!\n");
		exit(0);
	}


	// Create shared memeory
	int shmid;
	shmid = shmget(key, SIZE, IPC_CREAT|0666);
	if(shmid < 0)
	{
		printf("Sender create shared memory failed!\n");
		exit(0);
	}

	// shared memory map to program address space
	char *shmaddr;
	shmaddr = shmat(shmid, NULL, 0);

	if(shmaddr < 0)
	{
		printf("Sender map shared memory failed!\n");
		exit(0);
	}

	printf("Please input sender string:\n");
	char message[SIZE];
	scanf("%s", message);

	// Sender send content to Receiver
	sem_wait(mutex);
	strcpy(shmaddr, message);
	sem_post(mutex);
	printf("Sender sent content: %s\n", message);

	// Waiting for Receiver response
	char reply[SIZE];
	sem_wait(sync);
	strcpy(reply, shmaddr);
	printf("Sender received content: %s\n", reply);

	// Delete semaphore
	sem_close(mutex);
	sem_unlink(MUTEX_NAME);
	sem_close(sync);
	sem_unlink(SYNC_NAME);

	// Delete share memory
	shmdt(shmaddr);
	shmctl(shmid, IPC_RMID, 0);
	printf("Sender process end!\n");

	return 0;
}
