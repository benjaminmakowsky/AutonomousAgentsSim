#! /bin/bash
gcc -I defender.c libcAI.so -lpthread -lexpat -lz -lm -o defender 
