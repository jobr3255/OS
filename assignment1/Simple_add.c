#include <linux/kernel.h>
#include <linux/linkage.h>
asmlinkage long sys_cs3753_add(int num1, int num2, int *result){
	printk(KERN_INFO "Numbers to be added: %d, %d \n", num1, num2);
	*result = num1 + num2;
	printk(KERN_INFO "Result of addition: %d \n", *result);
	return 0;
}
