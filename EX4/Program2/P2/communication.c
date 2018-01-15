#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

#define READ 0
#define WRITE 1

int main()
{
	int fd[2];
	pid_t pid1,pid2,pid3;
	char buf[256];
	int returned_count;

	pipe(fd);

	if((pid1 = fork()) == -1)
	{
		printf("Error in fork pid1\n");
		exit(1);
	}
	if(pid1 == 0)
	{
		sleep(1);
		printf("Pid1: %d in the spawned (child) process\n", getpid());
		close(fd[READ]);
		char *string1 = "pid1 send string 1\n"; 
		write(fd[WRITE], string1, strlen(string1));
		exit(0);
	}

	if((pid2 = fork()) == -1)
	{
		printf("Error in fork pid2\n");
		exit(1);
	}
	if(pid2 == 0)
	{
		sleep(1);
		printf("Pid2: %d in the spawned (child) process\n", getpid());
		close(fd[READ]);
		char *string2 = "pid2 send string 2\n"; 
		write(fd[WRITE], string2, strlen(string2));
		exit(0);
	}

	if((pid3 = fork()) == -1)
	{
		printf("Error in fork pid3\n");
		exit(1);
	}
	if(pid3 == 0)
	{
		sleep(1);
		printf("Pid3: %d in the spawned (child) process\n", getpid());
		close(fd[READ]);
		char *string3 = "pid3 send string 3\n"; 
		write(fd[WRITE], string3, strlen(string3));
		exit(0);
	}else{
		pid1 = waitpid(pid1, NULL, WUNTRACED);
		pid2 = waitpid(pid2, NULL, WUNTRACED);
		pid3 = waitpid(pid3, NULL, WUNTRACED);
		printf("Parent process pid: %d\n", getpid());
		printf("Wait pid: %d %d %d in the spawning (parent) process...\n", pid1, pid2, pid3);
		close(fd[WRITE]);
		returned_count = (int)read(fd[READ], buf, sizeof(buf));
		printf("%d bytes of data received from spawned process: \n%s\n", returned_count, buf);
	}
	return 0;
}
