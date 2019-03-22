#!/bin/bash
dmesg --clear

echo ""; echo "-----------------------------------";
echo "Running helloworld.c with system call sys_helloworld:"
echo "-----------------------------------"; echo "";

gcc -o hello hello.c
./hello

echo ""; echo "-----------------------------------";
echo "Running cs3753add.c with system call sys_cs3753_add:"
echo "-----------------------------------"; echo "";

gcc -o add add.c
./add

echo ""; echo "---------------------";
echo "Showing dmesg output:"
echo "---------------------"; echo "";

dmesg

echo ""
