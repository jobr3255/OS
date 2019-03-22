#include <stdio.h>
#include <linux/kernel.h>
#include <sys/syscall.h>
#include <unistd.h>
int main(void) {
	int temp = 0;
	int *ptr = &temp;
	printf("Pointer value before system call: %d\n", *ptr);
	int num1 = 431;
	int num2 = 521;
	long int returnValue = syscall(334, num1, num2, ptr);
	printf("System call sys_cs3753_add returned %ld\n", returnValue);
	printf("Pointer value after system call: %d\n", *ptr);
  return 0;
}
