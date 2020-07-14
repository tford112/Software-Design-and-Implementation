
#include <stdio.h> 
#include <stdlib.h>
#include <string.h> 
#include "../include/indexer.h"
#include "../include/hash.h"
#include "../include/recreate.h" 

INVERTED_INDEX* recreateIndex(FILE* data, FILE* logger) { 
	char buf[BUFSIZE]; 
	memset(buf, '\0', BUFSIZE); 
	INVERTED_INDEX* index = malloc(sizeof(INVERTED_INDEX));
	if (index == NULL) {
		perror("Not enough memory. Exiting..\n");
		exit(EXIT_RETURN); 
	}
	for (int i = 0; i < MAX_HASH_SLOT; ++i) {
		index->hash[i] = NULL; 
	}
	int docCounter = 0; 
	int totalDocs = 0; 
	while (fgets(buf, BUFSIZE, data)) {
		char* split = strtok(buf, " "); 
		printf("Word is: \"%s\"", split); 
		fprintf(logger, "Word is: \"%s\"\n", split); 
		WordNode* wnode = malloc(sizeof(WordNode));
		if (wnode == NULL) {
			perror("Not enough memory\n");
			fputs("Not enough memory for wnode\n", logger); 
			exit(EXIT_RETURN); 
		}	
		wnode->page = NULL;
		wnode->next= NULL; 
		wnode->prev = NULL; 
		strlcpy(wnode->word, split, WORD_LENGTH); 
		split = strtok(NULL, " "); 
		totalDocs = atoi(split); 
		fprintf(logger, "Total docs: %d\n", totalDocs); 
		DocNode** allDocNodesPerWord = malloc(sizeof(DocNode*) * totalDocs);  // number of total dnodes to allocate for a word 
		if (allDocNodesPerWord == NULL) {
			perror("Couldn't allocate memory for array of DocNodes for word");
			fputs("Couldn't allocate memory for array of DocNodes for word", logger);
			exit(EXIT_RETURN); 
		}
		for (int i = 0; i < totalDocs; ++i) {
			allDocNodesPerWord[i] = malloc(sizeof(DocNode)); 
			if (allDocNodesPerWord[i] == NULL) {
				perror("Couldn't allocate memory for dnode"); 
				fprintf(logger, "Failed to allocate memory for docnode in array for word %s", wnode->word); 
				exit(EXIT_RETURN); 
			}
			allDocNodesPerWord[i]->next = NULL;  
		}
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
			printf("%s ", split); 
		}
		unsigned long hash_value = hash1(wnode->word) % MAX_HASH_SLOT; 
		if (index->hash[hash_value] == NULL) {
			wnode->page = allDocNodesPerWord[0]; 			// docNodes are now assigned to wnode 
			index->hash[hash_value] = wnode; 
			fputs("Added wnode to index\n", logger); 
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

int main(int argc, char** argv) {
	FILE* logger = fopen("recreate_index_logger.txt", "wb");
	if (logger == NULL){
		perror("Not enough memory to allocate. Exiting...\n"); 
		exit(EXIT_RETURN); 
	}
	FILE* data = fopen("sorted_index.dat", "r"); 
	if (data == NULL) {
		perror("Can't open file..\n");
		exit(EXIT_RETURN);
	}
	INVERTED_INDEX* test = recreateIndex(data, logger); 
	saveIndex(test, "test_index.dat", logger); 
	cleanUp(test, logger); 
	fclose(data);
	fclose(logger); 
}
