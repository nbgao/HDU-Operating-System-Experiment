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
char *SYNC_SENDER_NAME = "sender_done";		// sender已经发送消息令牌
char *SYNC_RECEIVER_NAME = "receiver_done";		// receiver已经接收消息令牌

int main()
{
	key_t key;		//标志共享内存的自定义键值
	key = 2018;

	sem_t *mutex;		// 互斥信号量
	sem_t *sender_done;		// sender令牌
	sem_t *receiver_done;		// receiver令牌

	// open mutex
	mutex = sem_open(MUTEX_NAME, O_CREAT, 0644, 1);
	if(mutex == SEM_FAILED)
	{
		printf("Receiver open mutex failed!\n");
		sem_unlink(MUTEX_NAME);
		perror("Error");
		exit(0);
	}

	// open receiver_done
	receiver_done = sem_open(SYNC_RECEIVER_NAME, O_CREAT, 0644, 0);
	if(receiver_done == SEM_FAILED)
	{
		printf("Sender create receiver_done failed!\n");
		sem_unlink(SYNC_RECEIVER_NAME);
		perror("Error");
		exit(0);
	}

	// open sender_done
	sender_done = sem_open(SYNC_SENDER_NAME, O_CREAT, 0644, 0);
	if(sender_done == SEM_FAILED)
	{
		printf("Sender create sender_done failed!\n");
		sem_unlink(SYNC_SENDER_NAME);
		perror("Error");
		exit(0);
	}


	// Create shared memeory
	int shmid;
	shmid = shmget(key, SIZE, IPC_CREAT|0666);
	if(shmid < 0)
	{
		printf("Sender create share memory failed!\n");
		perror("Error");
		exit(0);
	}

	// shared memory map to program address space
	char *shmaddr;
	shmaddr = shmat(shmid, NULL, 0);

	if(shmaddr < 0)
	{
		printf("Sender map share memory failed!\n");
		perror("Error");
		exit(0);
	}

	printf("Please input sender string:\n");
	char message[SIZE];
	scanf("%s", message);

	// Sender send content to Receiver
	sem_wait(mutex);
	strcpy(shmaddr, message);
	sem_post(mutex);
	sem_post(sender_done);
	printf("Sender sent content: %s\n", message);

	// Waiting for Receiver response
	char reply[SIZE];
	sem_wait(receiver_done);
	strcpy(reply, shmaddr);
	printf("Sender receive content: %s\n", reply);

	// Delete semaphore
	sem_close(mutex);
	sem_unlink(MUTEX_NAME);
	sem_close(sender_done);
	sem_unlink(SYNC_SENDER_NAME);
 	sem_close(receiver_done);
	sem_unlink(SYNC_RECEIVER_NAME);

	// Delete share memory
	shmctl(shmid, IPC_RMID, 0);
	printf("Sender process end!\n");

	return 0;
}
