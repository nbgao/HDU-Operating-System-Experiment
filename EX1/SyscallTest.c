#define _GNU_SOURCE
#include <unistd.h>
#include <sys/syscall.h>
#define __NR_gpbsyscall 333

int main()
{
	syscall(__NR_gpbsyscall);
}
