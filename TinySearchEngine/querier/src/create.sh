#!/bin/bash

# file builds anew and moves executable to bin and cleans up 

make clean
make 
#sed '/^$/d' index.dat > no_emplines_index.dat ## remove empty lines to avoid segfault in ./recreate 
echo "finished compiling" 
echo "moving 'querier' executable to bin" 
mv querier ../bin 
echo "cleaning up" 
make clean 
echo "finished"
