#!/bin/bash

# execute in order 
# 1. "./indexer urls" 
# 2. "./indexer texts"
# 3. open index.dat remove first line 
# 4. "./recreate" 

make clean
make 
./indexer texts 
sed '/^$/d' index.dat > no_emplines_index.dat ## remove empty lines to avoid segfault in ./recreate 
echo "recreating index.." 
./recreate 
echo "sorting indices.."
sort index.dat > sorted_index.dat
sort test_index.dat > sorted_test_index.dat 
diff sorted_index.dat sorted_test_index.dat 
if [ $? -ne 0 ]; 
then 
	echo "indices do not match" 
else 
	echo "Parsed index and recreated index match" 
fi 
echo "finished"
