#include <unistd.h>
#include <sys/syscall.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#define mysetnice 334

int main()
{
	int pid, nicevalue, prio, nice, s;

	printf("Please input PID:\n");
	scanf("%d", &pid);

	printf("Get nice(0) or Set nice(1)\n");
	do{
		scanf("%d", &s);
	}while(s!=0 && s!=1);

	if(s==0){
		if(syscall(mysetnice, pid, 0, 0, &prio, &nice)){
			printf("Fail to get prio!");
		}else{
			printf("Current prio: %d\n", prio);
		}
	}else if(s==1){
		printf("Please input nice value:\n");
		scanf("%d", &nicevalue);
		while(nicevalue<-20 || nicevalue>19){
			printf("The nice value is out of limitation !\n");
			printf("Please input nice value again:\n");
			scanf("%d", &nicevalue);
		}
		//printf("Old nice: %d\n", nicevalue);
		syscall(mysetnice, pid, 1, nicevalue, &prio, &nice);
		printf("New nice: %d\n", nice);
		printf("New prio: %d\n", prio);
	}

	return 0;
}
