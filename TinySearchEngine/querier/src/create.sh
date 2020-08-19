#!/bin/bash

# execute in order 
# 1. "./indexer urls" 
# 2. "./indexer texts"
# 3. open index.dat remove first line 
# 4. "./recreate" 
# 5. check whether recreated index and original differ 
# 6. If not, then move the original and executables to bin 
# 7. clean up 

make clean
make 
sed '/^$/d' index.dat > no_emplines_index.dat ## remove empty lines to avoid segfault in ./recreate 
sort no_emplines_index.dat > sorted_index.dat 
echo "finished"
