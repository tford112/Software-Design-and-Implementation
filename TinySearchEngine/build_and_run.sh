#!/bin/bash 

## run crawler, indexer, and querier directory shell scripts that will build the respective executables and pass them along to the next 
## 1. building the crawler will create the executable that will fill the target directory with specially-formatted text files (each file will have the URL used to download the HTML page, a document Id which is the same name as the text file, and the actual HTML that was downloaded) 
## 2. The target directory ("urls") will be copied to the indexer/src directory. The indexer will first extract all the relevant text from the HTML for each file and store them into text files. After, the indexer will create an index.dat file that will store "dictionary" of words whose definitions are really the # of documents they appear in as well as their word frequencies. A functional test is conducted by recreating the INVERTED_INDEX* data structure from the outputted index text file and saving this recreated index as another .dat file to be matched with the original. If they match, we proceed 
## 3. Finally, we copy the sorted version of our index.dat file to the querier along with the "urls" directory (our "target directory" from crawler) and the texts directory that contains our text files. We need the "urls" directory to get the actual url name that will be printed out to the user as part of our query results. We need the "texts" directory because we will be requesting the user to select a document id from the query results to open the text file to read their selection as well as counting the frequencies a certain word appears in that text file. 

cd crawler/src
./execute_crawler.sh 
cd ../../indexer/src 
./execute_indexer.sh
cd ../../querier/src
./execute_querier.sh 
echo "Finished build..Now running querier" 
cd ../bin/ 
./querier ./final_index.dat 
echo "\n To run more queries, please go to querier/bin and run './querier final_index.dat'" 
