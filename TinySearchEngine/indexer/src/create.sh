#!/bin/bash

# 1. "./indexer urls" -> extract text from all URLs (uses python to do BeautifulSoup Extraction) 
# 2. "./indexer texts"-> parse the text files to create a search index 
# 3. open index.dat remove first line 
# 4. "./recreate"  -> recreate the index from the index.dat (with no empty lines) 
# 5. check whether recreated index and original differ 
# 6. If not, then we can sort our index.dat into a "sorted_index.dat" which will be copied to the "querier" directory 
# 7. Move the rest of the executables to bin 
# 8. clean up

make clean
make 
echo "Extracting text from URL files..." 
./indexer urls   ## run indexer to get text files from urls (will be stored in "texts" directory) 
echo "Done!\nParsing text files to create index" 
./indexer texts 
#sed '/^$/d' index.dat > no_emplines_index.dat ## remove empty lines to avoid segfault in ./recreate 
echo "Done!\nNow recreating index and check for differences with original" 
echo "recreating index..."
./recreate 			## outputs a "test_index.dat" 
echo "sorting indices.."
sort index.dat > sorted_index.dat
sort test_index.dat > sorted_test_index.dat    
diff sorted_index.dat sorted_test_index.dat 
if [ $? -ne 0 ]; 
then 
	echo "indices do not match" 
else 
	echo "Parsed index and recreated index match" 
	echo "Copying sorted_index.dat to querier bin directory" 
	cp sorted_index.dat ../../querier/bin
	mv sorted_index.dat ../bin/
	mv indexer ../bin/
	mv recreate ../bin/ 
	echo "Moved index and executables to bin" 
fi 
make clean 
echo "finished"
