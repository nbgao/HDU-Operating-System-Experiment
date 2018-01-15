#include <pthread.h>
#include <semaphore.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <unistd.h>
#include <sys/msg.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/stat.h>
#include <errno.h>
#include <pthread.h>

#define TRUE 1
#define BUF_SIZE 256
#define PERM S_IRUSR|S_IWUSR
#define KEY_NUM 2000


struct msgbuf{
	long mtype;
	char mtext[BUF_SIZE];
};


// semaphore
sem_t full;		// message queue has message
sem_t empty;	// message queue is empty
sem_t mutex;	// message mutex access
sem_t sender_over;	//receiver sent over


// pthread
pthread_t sender_tid;
pthread_t receiver_tid;

//key_t key;

// message
int msgid;

void Init()
{
	// Initialize Semaphore
	sem_init(&full,0,0);
	sem_init(&empty,0,1);
	sem_init(&mutex,0,1);
	sem_init(&sender_over,0,0);

	// Create Message Queue
	if((msgid = msgget(KEY_NUM, PERM|IPC_CREAT)) == -1)
	{
		perror("错误");
	}

}


void *SenderThread(void *arg)
{
	char input[BUF_SIZE];
	struct msgbuf msg;
	msg.mtype = 1;
	int length = sizeof(struct msgbuf) - sizeof(long);

	while(TRUE)
	{
		sem_wait(&empty);
		sem_wait(&mutex);
		//sleep(0.1);
		printf("\nSenderThread: Please input the message to send:\n");
		scanf("%s", input);

		// Detect for exit
		if(strcmp(input, "exit") == 0)	// exit
		{
			strncpy(msg.mtext, "end", BUF_SIZE);
			msgsnd(msgid, &msg, length, 0);
			printf("SenderThread: [message sent] %s\n", msg.mtext);

			sem_post(&mutex);
			sem_post(&full);
			break;
		}

		strncpy(msg.mtext, input, BUF_SIZE);
		if(msgsnd(msgid, &msg, length, 0) < 0)
			perror("错误");

		printf("SenderThread: [message sent] %s\n", msg.mtext);

		// Semaphore
		sem_post(&mutex);		
		sem_post(&full);
		
	}

	sem_wait(&sender_over);
	sem_wait(&full);
	sem_wait(&mutex);

	msgrcv(msgid, &msg, length, 1, 0);

	printf("SenderThread: [message received] %s\n", msg.mtext);
	sem_post(&mutex);
	sem_post(&empty);

	// Remove message queue
	if(msgctl(msgid, IPC_RMID, 0) == -1)
	{
		printf("Remove message queue error!\n");
		exit(0);
	}
	pthread_exit(0);
}



void *ReceiverThread(void *arg)
{
	struct msgbuf msg;
	msg.mtype = 1;
	int length = sizeof(struct msgbuf) - sizeof(long);
	while(TRUE)
	{
		sem_wait(&full);
		sem_wait(&mutex);

		// Receive message from message queue
		if(msgrcv(msgid, &msg, length, 1, 0) < 0)
			perror("错误");

		// Detect for end
		if(strcmp(msg.mtext, "end") == 0)  // end
		{
			//msg.mtype = 2;
			strncpy(msg.mtext, "over", BUF_SIZE);
			printf("ReceiverThread: [message received] %s\n", msg.mtext);

			msgsnd(msgid, &msg, length, 0);
			sem_post(&sender_over);
			sem_post(&full);
			sem_post(&mutex);

			break;
		}

		// Print message
		printf("ReceiverThread: [message received] %s\n", msg.mtext);

		sem_post(&empty);
		sem_post(&mutex);
	}
	pthread_exit(0);
}



int main()
{
	// Initial Message Queue
	Init();

	// Create Thread
	if(pthread_create(&sender_tid, NULL, SenderThread, NULL) < 0)
	{
		printf("Create Thread Failed!\n");
		exit(0);
	}

	if(pthread_create(&receiver_tid, NULL, ReceiverThread, NULL) < 0)
	{
		printf("Create Thread Failed!\n");
		exit(0);
	}


	// Waiting for the thread end
	pthread_join(sender_tid, NULL);
	pthread_join(receiver_tid, NULL);

	sem_destroy(&full);
	sem_destroy(&empty);
	sem_destroy(&mutex);
	sem_destroy(&sender_over);

	printf("Main Thread End!\n");

	return 0;
}
