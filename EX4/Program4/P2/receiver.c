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
	mutex = sem_open(MUTEX_NAME, O_CREAT);
	if(mutex == SEM_FAILED)
	{
		printf("Receiver open mutex failed!\n");
		sem_unlink(MUTEX_NAME);
		perror("Error");
		exit(0);
	}

	// open sender_done
	sender_done = sem_open(SYNC_SENDER_NAME, O_CREAT);
	if(sender_done == SEM_FAILED)
	{
		printf("Receiver open sender_done failed!\n");
		sem_unlink(SYNC_SENDER_NAME);
		perror("Error");
		exit(0);
	}

	// open receiver_done
	receiver_done = sem_open(SYNC_RECEIVER_NAME, O_CREAT);
	if(receiver_done == SEM_FAILED)
	{
		printf("Receiver open receiver_done failed!\n");
		sem_unlink(SYNC_RECEIVER_NAME);
		perror("Error");
		exit(0);
	}

	// Open shared memory
	int shmid;
	shmid = shmget(key, SIZE, 0666);
	if(shmid < 0)
	{
		printf("Receiver open share memory failed!\n");
		perror("Error");
		exit(0);
	}

	// shared memory map to program address space
	char *shmaddr;
	shmaddr = shmat(shmid, NULL, 0);

	// Receive content of Sender
	char *message;
	sem_wait(sender_done);
	sem_wait(mutex);
	strcpy(message, shmaddr);

	sem_post(mutex);
	printf("Receiver get message: %s\n", message);

	// Receiver send over
	char *reply = "over";
	sem_wait(mutex);
	strcpy(shmaddr, reply);
	sem_post(mutex);

	// Receiver release token
	sem_post(receiver_done);
	sem_close(mutex);
	sem_close(receiver_done);
	sem_close(sender_done);

	printf("Receiver process end!\n");

	return 0;
}
