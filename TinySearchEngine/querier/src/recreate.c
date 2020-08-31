
#include <stdio.h>
#include <stdlib.h>
#include "../include/query.h" 
#include "../include/hash.h"
#include "../include/recreate.h" 
#include "../include/allocate.h" 
#include "../include/clean.h"

/* FILE: recreate.c 
 *
 * DESCRIPTION: From our .dat index file that our main indexer section created, we now have to recreate our 
 * INVERTED_INDEX data structure that can hold all our information about the words and the documents all 
 * of them appear in. The output is a fully recreated INVERTED_INDEX* structure that is used for our query 
 * functions to figure out the, well, queries.
 */

INVERTED_INDEX* returnRecreatedIndex(FILE* data, FILE* logger) { 
	INVERTED_INDEX* index = recreateIndex(data, logger); 
	return index; 
}

// Because of how our .dat file is formatted (reminder: # of total docs appears after the immmediate word,
// and each pair of numbers afterwards indicates the document it appears in and how many times. e.g. we 
// could have "acting 2 24 1 32 2" which means the word "acting" appears in only 2 documents: 24 and 32. 
// It appears only once in document 24, which is really "text_24", and twice in document 32) we have to
// use strtok() to continually break each line in our file into pieces that will be handled differently. 
// We create an array of DocNodes that will track all this information. After we're done, we then set 
// allocate a WordNode and set it's pointer to point to the first element in our docNode array generated 
// for this word. We then insert the word node in our hash table (if there is a hash collision, we create 
// a linked list at that spot in our hash table) 
INVERTED_INDEX* recreateIndex(FILE* data, FILE* logger) { 
	char buf[BUFSIZE] = {0}; 
	INVERTED_INDEX* index = allocateInvertedIndex(data); 
	int docCounter = 0; 
	int totalDocs = 0; 
		
	while (fgets(buf, BUFSIZE, data)) {     		        // each line in our .dat file will have to go through this entire process
		char* split = strtok(buf, " "); 
		fprintf(logger, "Word is: \"%s\"\n", split); 
		WordNode* wnode = allocateWordNode(logger); 
		strlcpy(wnode->word, split, WORD_LENGTH); 
		split = strtok(NULL, " "); 
		totalDocs = atoi(split); 
		fprintf(logger, "Total docs: %d\n", totalDocs); 
		DocNode* allDocNodesPerWord = allocateDocNodeArray(totalDocs); 
		int split_count = 0; 
		while (split && docCounter < totalDocs) {
			split = strtok(NULL, " "); 
			if (split) {
				if (split_count % 2 == 0 ) {  		// even numbers represent the text docIds 
					allDocNodesPerWord[docCounter].docId = atoi(split); 
					fprintf(logger, "added docId %d to docNodes array\n", allDocNodesPerWord[docCounter].docId); 
					if (docCounter < totalDocs - 1) {
						allDocNodesPerWord[docCounter].next = &allDocNodesPerWord[docCounter+1]; 
						fprintf(logger, "currDocCounter: %d set connection to next node: %d\n", docCounter, docCounter+1); 
					}
				}
				else {
					allDocNodesPerWord[docCounter].page_word_frequency = atoi(split); 
					fprintf(logger, "word frequency %d for current doc\n\n", allDocNodesPerWord[docCounter].page_word_frequency);
					++docCounter; 
				}
			}
			++split_count; 
		}
		unsigned long hash_value = hash1(wnode->word) % MAX_HASH_SLOT; 
		if (index->hash[hash_value] == NULL) {
			wnode->page = &allDocNodesPerWord[0]; 		// docNodes are now assigned to wnode 
			index->hash[hash_value] = wnode; 
			fputs("\nAdded wnode to index\n", logger); 
		}
		else { 
			WordNode* currWord = index->hash[hash_value];  
			while (currWord->next != NULL) { 		// collision occurred in hash table. Add new word at end of hash list 
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

