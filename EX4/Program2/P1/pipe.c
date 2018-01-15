#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#define size 40
int fd[2]; // fd[0]读管道，fd[1]写管道
void WritePipe(int id){
	close(fd[0]);
	lockf(fd[1],1,0);
	printf("child process %d, pid = %d, start write.\n",id+1, getpid());
	char mesg[100];
	sprintf(mesg,"this message from child %d.\n",id+1);
	int re=(int)write(fd[1],mesg,size);
	if(re != -1){
		printf("child process %d 写入完毕,写入的字节数为：%d\n",id+1,re);
	}else{
		printf("child process %d 写入失败\n",id+1);
	}
	sleep(1);
	lockf(fd[1],0,0);
	exit(0);
}

int main(){
	pid_t pid[3];
	char mesgs[40];
	int i;
	if(pipe(fd)<0) //创建管道失败
	{
		fprintf(stderr,"create pipe error:%s\n",strerror(errno));
	}
	if((pid[0]=fork())<0){
		fprintf(stderr,"fork 1 error:%s\n",strerror(errno));
		exit(EXIT_FAILURE);
	}
	if(pid[0]==0){
		WritePipe(0);
	}
	else
	{
		if((pid[1]=fork())<0){
			fprintf(stderr,"fork 2 error:%s\n",strerror(errno));
			exit(EXIT_FAILURE);
		}
		if(pid[1]==0){
			WritePipe(1);
		}
		else
		{
			if((pid[2]=fork())<0){
				fprintf(stderr,"fork 3 error:%s\n",strerror(errno));
				exit(EXIT_FAILURE);
			}
			if(pid[2]==0){
				WritePipe(2);
			}else{
				wait(0);
				wait(0);
				wait(0);
				close(fd[1]);

				printf("\nparent process %d, start read:\n",getpid());
				for(i=0;i<3;i++){
					if(read(fd[0],mesgs,size)!=-1){
						printf("%s",mesgs);
					}else{
						printf("读取子进程%d失败!\n",i);
					}
				}
				printf("It's end!\n");
				return 0;
			}
		}
	}
	return 0;
}
