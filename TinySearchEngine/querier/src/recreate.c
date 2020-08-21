
#include <stdio.h>
#include <stdlib.h>
#include "../include/query.h" 
#include "../include/hash.h"
#include "../include/recreate.h" 
#include "../include/allocate.h" 
#include "../include/saveClean.h"

INVERTED_INDEX* returnRecreatedIndex(FILE* data, FILE* logger) { 
	INVERTED_INDEX* index = recreateIndex(data, logger); 
	return index; 
}

// We convert our "index.dat" into an INVERTED_INDEX for our querier to use and search against. 
// We have to keep using strtok() to continually break up our line into multiple pieces each of which 
// will need to be handled differently based on the index of the split word. The first "split" after 
// strtok will be the word itself. The second strtok will be the # of documents that word appears in. 
// The rest of the splits are the text documents and the number of times that word appears in that 
// specific text doc. 
// For instance, the first line might be "contract 3 28 1 31 1 40 2". This means the word "contract" 
// appears in 3 documents in total. It appears in document 28 1 time, document 31 1 time, and document 
// 40 two times. 
INVERTED_INDEX* recreateIndex(FILE* data, FILE* logger) { 
	char buf[BUFSIZE] = {0}; 
	fgets(buf, BUFSIZE, data); 
	if (strcmp(buf, "\n") != 0) {
		fputs("First line was not empty\n", logger);
	}
	memset(buf, 0, BUFSIZE); 
	INVERTED_INDEX* index = allocateInvertedIndex(data); 
	int docCounter = 0; 
	int totalDocs = 0; 
	while (fgets(buf, BUFSIZE, data)) {
		char* split = strtok(buf, " "); 
		fprintf(logger, "Word is: \"%s\"\n", split); 
		WordNode* wnode = allocateWordNode(logger); 
		strlcpy(wnode->word, split, WORD_LENGTH); 
		split = strtok(NULL, " "); 
		totalDocs = atoi(split); 
		fprintf(logger, "Total docs: %d\n", totalDocs); 
		DocNode** allDocNodesPerWord = allocateDocNodeArray(logger, totalDocs); 
		int split_count = 0; 
		while (split && docCounter < totalDocs) {
			split = strtok(NULL, " "); 
			if (split) {
				if (split_count % 2 == 0 ) {  // even numbers represent the text docIds 
					allDocNodesPerWord[docCounter]->docId = atoi(split); 
					fprintf(logger, "added docId %d to docNodes array\n", allDocNodesPerWord[docCounter]->docId); 
					if (docCounter < totalDocs - 1) {
						allDocNodesPerWord[docCounter]->next = allDocNodesPerWord[docCounter+1]; 
						fprintf(logger, "currDocCounter: %d set connection to next node: %d\n", docCounter, docCounter+1); 
					}
				}
				else {
					allDocNodesPerWord[docCounter]->page_word_frequency = atoi(split); 
					fprintf(logger, "word frequency %d for current doc\n\n", allDocNodesPerWord[docCounter]->page_word_frequency);
					++docCounter; 
				}
			}
			++split_count; 
		}
		unsigned long hash_value = hash1(wnode->word) % MAX_HASH_SLOT; 
		if (index->hash[hash_value] == NULL) {
			wnode->page = allDocNodesPerWord[0]; 			// docNodes are now assigned to wnode 
			index->hash[hash_value] = wnode; 
			fputs("\nAdded wnode to index\n", logger); 
		}
		else { 
			WordNode* currWord = index->hash[hash_value];  
			while (currWord->next != NULL) { 			// collision occurred in hash table. Add new word at end of hash list 
				currWord = currWord->next; 
			}	
			fputs("Collision occurred at same hash value..iterated through DocNode list. Now adding at end\n", logger); 
			currWord->next = wnode; 
			wnode->prev = currWord; 
		}
		docCounter = 0; 
		memset(buf, '\0', BUFSIZE); 
	}
	return index; 
}

