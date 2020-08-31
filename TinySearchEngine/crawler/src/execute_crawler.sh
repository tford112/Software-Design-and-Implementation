#!/bin/bash

# make build anew for crawler and test on JHU seed url. Move the crawler executable to bin and copy the 
# URLS generated to the Indexer so it can start its own process therea

## NOTE: If the seed URL changes from https://www.jhu.edu, then the command below "./crawler [SEED_URL] urls" will need to change 

make clean
make 
./crawler https://www.jhu.edu urls 3   ## run crawler if SEED_URL is www.jhu.edu 
mv crawler ../bin 
echo "Moved crawler executable to bin"
cp -r urls ../../indexer/src/
echo "Copied target url directory to 'indexer' directory" 
make clean 
echo "finished"
