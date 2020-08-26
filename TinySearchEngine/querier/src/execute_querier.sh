#!/bin/bash

make clean
make 
echo "copy the texts and move querier to bin directory" 
mv querier ../bin/
make clean
echo "finished"

