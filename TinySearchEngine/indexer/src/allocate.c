
#include <stdio.h> 
#include <stdlib.h> 
#include "../include/indexer.h" 

FILE* openFile(char* filename, char* mode) {
	FILE* out = fopen(filename, mode); 
	if (out == NULL) {
		perror("Can't open file. Exiting...\n");
		exit(EXIT_RETURN); 
	}
	return out; 
}

INVERTED_INDEX* allocateInvertedIndex(FILE*log) {
	INVERTED_INDEX* index = malloc(sizeof(INVERTED_INDEX)); 
	if (index == NULL) {
		perror("Not enough memory for index. Exiting..\n"); 
		fprintf(log, "Not enough memory for index. Exiting..\n"); 
		exit(1);
	}
	for (int i = 0; i < MAX_HASH_SLOT; ++i) {
		index->hash[i] = NULL; 
	}
	return index; 
}

WordNode* allocateWordNode(FILE* log) {
	WordNode* wnode = malloc(sizeof(WordNode)); 
	if (wnode == NULL) {
		perror("Couldn't allocate memory for word\n");
		fputs("Couldn't allocate memory for word\n", log); 
		exit(EXIT_RETURN); 
	}
	wnode->next = NULL;
	wnode->prev = NULL; 
	wnode->page = NULL; 
	return wnode; 
}

DocNode* allocateDocNode(FILE* log) {
	DocNode* dnode = malloc(sizeof(DocNode)); 
	if (dnode == NULL) {
		perror("Couldn't allocate memory for docnode\n");
		fputs("Couldn't allocate memory for docnode\n", log); 
		exit(EXIT_RETURN); 
	}
	dnode->next = NULL; 
	dnode->docId = -1; 
	dnode->page_frequency = 0; 
	return dnode; 
}

DocNode** allocateDocNodeArray(FILE* log, int totalDocs) { // number of total dnodes to allocate for a word  
	DocNode** allDocNodesPerWord = malloc(sizeof(DocNode*) * totalDocs);  
	if (allDocNodesPerWord == NULL) {
		perror("Couldn't allocate memory for array of DocNodes for word");
		fputs("Couldn't allocate memory for array of DocNodes for word", log);
		exit(EXIT_RETURN); 
	}
	for (int i = 0; i < totalDocs; ++i) {
		allDocNodesPerWord[i] = allocateDocNode(log); 
	}
	return allDocNodesPerWord; 
}

