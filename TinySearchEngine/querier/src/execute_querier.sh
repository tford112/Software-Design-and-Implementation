#!/bin/bash

## Shell script that cleans and builds the querier

make clean
make 
echo "move querier to bin directory" 
mv querier ../bin/
make clean
echo "finished"

