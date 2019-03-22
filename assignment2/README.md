### Josh Brown

# Files

* README.md
* simple_char_driver.c
* test_simple_char_device_file.c

# Steps

###### create inode
sudo mknod -m 777 /dev/simple_character_device c 244 0

###### add to Makefile
obj-m:=simple_char_driver.o

###### make modules
sudo make -C /lib/modules/$(uname -r)/build M=$PWD modules

###### install module
sudo insmod simple_char_driver.ko

###### remove module
sudo rmmod simple_char_driver

###### compile test code
gcc -o test_simple_char_device_file test_simple_char_device_file.c

###### run test code
./test_simple_char_device_file
