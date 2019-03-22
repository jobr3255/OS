#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/slab.h>

#include <linux/init.h>
#include <linux/module.h>

MODULE_LICENSE("GPL");
#define BUFFER_SIZE 1024
#define  DEVICE_NAME "simple_char_driver"

/*
All variables are initialized upon insmod and are not reset unless
the module is removed and reinstalled in the kernel
*/
int counterOpen = 0;
int counterClose = 0;

char *device_buffer;
// keeps track of the major number
static int majorNumber;

int currentFilePosition = 0;
int return_error;
ssize_t simple_char_driver_read (struct file *pfile, char *buffer, size_t length, loff_t *offset)
{
    printk(KERN_ALERT "inside %s function\n",__FUNCTION__);
    printk(KERN_ALERT "Current offset = %d", (int)*offset);

    // bound check for offset
    if(*offset >= BUFFER_SIZE)
      *offset = BUFFER_SIZE - 1;

    // bound check for length
    if((*offset + length) >= BUFFER_SIZE)
      length = (BUFFER_SIZE - *offset - 1);

    // copy_to_user( * to, *from, size) returns 0 on success
    return_error = copy_to_user(buffer, &device_buffer[*offset], length);

    if (return_error == 0){
       printk(KERN_ALERT "%s: Sent %lu bytes\n", DEVICE_NAME, length);
       *offset += length;
       return length;
    } else {
       printk(KERN_ALERT "%s: Failed to send %lu bytes\n", DEVICE_NAME, length);
       return -EFAULT;
    }
}

ssize_t simple_char_driver_write (struct file *pfile, const char __user *buffer, size_t length, loff_t *offset)
{
    printk(KERN_ALERT "inside %s function\n",__FUNCTION__);

    printk(KERN_ALERT "Current offset = %d", (int)*offset);

    // bound check for offset
    if(*offset >= BUFFER_SIZE)
      *offset = BUFFER_SIZE - 1;

    // bound check for length
    if((*offset + length) >= BUFFER_SIZE)
      length = (BUFFER_SIZE - *offset - 1);

    //copy_from_user(destination, source, size)
    copy_from_user(&device_buffer[*offset], buffer, length);

    printk(KERN_ALERT "%s: Wrote %lu bytes\n", DEVICE_NAME, length);

    *offset += length;

    return length;
}

loff_t simple_char_driver_llseek (struct file *pfile, loff_t offset, int whence)
{
    printk(KERN_ALERT "inside %s function\n",__FUNCTION__);
    switch(whence){
        // the current position is set to the value of the offset
        case 0:  // SEEK_SET
            pfile->f_pos = offset;
            break;
        // the current position is incremented by offset bytes
        case 1: // SEEK_CUR
            pfile->f_pos = pfile->f_pos + offset;
            break;
        // the current position is set to offset bytes before the end of the file
        case 2: // SEEK_END
            pfile -> f_pos = strlen(&device_buffer[0]) - offset;
            break;
    }
    return 0;
}

int simple_char_driver_open(struct inode *pinode, struct file *pfile)
{
    counterOpen ++;
    printk(KERN_ALERT "simple_char_driver has been opened %d times\n", counterOpen);
    return 0;
}

int simple_char_driver_close(struct inode *pinode, struct file *pfile)
{
    counterClose ++;
    printk(KERN_ALERT "simple_char_driver been closed %d times\n", counterClose);
    return 0;
}

struct file_operations simple_char_file_operation = {
   .owner = THIS_MODULE,
   .read = simple_char_driver_read,
   .write = simple_char_driver_write,
   .open = simple_char_driver_open,
   .llseek = simple_char_driver_llseek,
   .release = simple_char_driver_close,
};

static int simple_char_driver_init(void)
{
    printk(KERN_ALERT "inside %s function\n",__FUNCTION__);

    // register device number
    majorNumber = register_chrdev(0, DEVICE_NAME, &simple_char_file_operation);

    printk(KERN_ALERT "simple_char_driver: registered correctly with major number %d\n", majorNumber);

    device_buffer = kmalloc((size_t)BUFFER_SIZE, GFP_KERNEL);
    return 0;
}
static void simple_char_driver_exit(void)
{
    //The cleanup function has no value to return, so it is declared void
    printk(KERN_ALERT "inside %s function\n",__FUNCTION__);

    unregister_chrdev(majorNumber, DEVICE_NAME);

    // free memory
    kfree(device_buffer);
}

module_init(simple_char_driver_init);
module_exit(simple_char_driver_exit);
