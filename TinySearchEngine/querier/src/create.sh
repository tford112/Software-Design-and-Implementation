#!/bin/bash

# file builds anew and moves executable to bin and cleans up 

make clean
make 
echo "finished compiling" 
echo "moving 'querier' executable to bin" 
mv querier ../bin 
echo "cleaning up" 
make clean 
echo "finished"
