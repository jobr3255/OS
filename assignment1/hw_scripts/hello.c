#include <stdio.h>
#include <linux/kernel.h>
#include <sys/syscall.h>
#include <unistd.h>
int main(void) {
	long int shoutout = syscall(333);
	printf("System call sys_hello returned %ld\n", shoutout);
  	return 0;
}