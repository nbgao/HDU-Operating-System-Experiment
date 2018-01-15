#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#define MAX_CMD_LEN 40
#define CMD_COLLECTION_LEN 4

#define INVALID_COMMAND -1
#define EXIT 0
#define CMD1 1
#define CMD2 2
#define CMD3 3

#define TRUE 1

char *cmdStr[CMD_COLLECTION_LEN] = {"exit", "cmd1", "cmd2", "cmd3"};

int getCmdIndex(char *cmd)
{
	int i;
	for(i=0;i<CMD_COLLECTION_LEN;i++)
	{
		if(strcmp(cmd, cmdStr[i]) == 0)
		{
			return i;
		}
	}
	return -1;
}

void myFork(int cmdIndex)
{
	pid_t pid;
	if((pid=fork()) < 0)
	{
		printf("Fork error\n");
		exit(0);
	}else if(pid==0){
		int execl_status = -1;
		//printf("Child is running\n");
		switch(cmdIndex)
		{
			case CMD1:
				execl_status = execl("./cmd1", "cmd1", NULL); break;
			case CMD2:
				execl_status = execl("./cmd2", "cmd2", NULL); break;
			case CMD3:
				execl_status = execl("./cmd3", "cmd3", NULL); break;
			default:
				printf("Invalid Command!\n"); break;
		}
		if(execl_status < 0)
		{
			printf("Fork error\n");
			exit(0);
		}
		printf("Fork success\n");
		exit(0);
	}else{
		return;
	}
}

void runCMD(int cmdIndex)
{
	switch(cmdIndex)
	{
		case INVALID_COMMAND:
			printf("Command not found!\n");
			break;
		case EXIT:
			exit(0);
			break;
		default:
			myFork(cmdIndex);
			break;
	}
}

int main()
{
	pid_t pid;
	char cmdStr[MAX_CMD_LEN];
	int cmdIndex;
	while(TRUE){
		printf("\nInput your commond > ");
		scanf("%s", cmdStr);
		cmdIndex = getCmdIndex(cmdStr);
		runCMD(cmdIndex);
		wait(0);	//父进程要等到子进程结束，才接收下一条命令
		printf("Waiting for next command ...");
	}
	return 0;
}
