
/*
 *  1. char loadDocument(char* filename) -> load HTML document from a file from the TARGET_DIRECTORY from crawler output 
 *
 *  2. int getDocumentId(char* filename) -> generate doc identifier from file name. 
 *
 *  4. int updateIndex(char* word, int docId, INVERTED_INDEX* index) -> updates data structure containing the index. Receives as input the string 
 *     containing the word and docID. Returns 1 if successful or else 0 
 *
 *  5. int saveFile(INVERTED_INDEX* index) -> saves the inverted index to a file. Receives pointer to inverted index as input
 *     returns 1 if successful else 0 
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
//#include "loadDoc.h"  // doesn't work at the moment...
#include "../include/loadDoc.h"
#include "../include/indexer.h" 
#include "../include/hash.h" 
#define MAX_FILE_NUM 25   // used for text extraction 

// get doc id from filename. We assume that the crawler saves files using progressive numbers as unique identifiers. 
int getDocumentId(char* filename) {  // don't extract the actual "text_" part  
	char* underscore = strchr(filename, '_'); // get first occurrence of '_' 
	int idx = underscore - &filename[0];  // get the index where '_' occurred
	char num[5]; 
	memcpy(num, &filename[idx+1], 4); 
	num[4] = '\0'; 
	printf("num is %s\n", num);
	return atoi(num); 
}

INVERTED_INDEX* initInvertedIndex(FILE*log) {
	INVERTED_INDEX* index = malloc(sizeof(INVERTED_INDEX)); 
	if (index == NULL) {
		perror("Not enough memory. Exiting..\n"); 
		fprintf(log, "Not enough memory. Exiting..\n"); 
		exit(1);
	}
	for (int i = 0; i < MAX_HASH_SLOT; ++i) {
		index->hash[i] = NULL; 
	}
	return index; 
}


void executeExtraction(FILE* log, char* dir) {
	int nfiles = numFiles(dir); 
	int count = 0; 
	fprintf(log, "Num files: %d\n", nfiles);
	char numToString[MAX_FILE_NUM];
	memset(numToString, 0, MAX_FILE_NUM); 
	fprintf(log, "running text extraction from HTMLS...\n"); 
	while (count < nfiles - 1) {
		snprintf(numToString, MAX_FILE_NUM, "%s/%d", dir, count); // get "string" from num and pass as argument to loadDocument  
		loadDocument(numToString);  		      
		fprintf(log, "extracted file %s\n", numToString); 
		memset(numToString, 0, MAX_FILE_NUM); 		      // good practice to clear out buffer 
		++count; 
	}
}

int checkWordInvalid(char* word) {
	int invalid = 0; 
	for (int i = 0; i < strlen(word); ++i) {
		if (!isalpha(word[i])) {
			++invalid;
		}
		if (strstr(word, "-")) {
			++invalid; 
		}
	}
	return invalid;
}

int updateIndex(char* word, int docId, INVERTED_INDEX* index, FILE* logger) {
	unsigned long hash_value; 
	hash_value = hash1(word) % MAX_HASH_SLOT;
	if (index->hash[hash_value] == NULL) {      // case where no instance of word found in hash table 
		WordNode* wnode = malloc(sizeof(WordNode)); 
		if (wnode == NULL) {
			perror("Not enough memory.\n");
			fprintf(logger, "Not enough memory.\n"); 
			exit(1);
		}	
		DocNode* dnode = malloc(sizeof(DocNode)); 
		if (dnode == NULL) {
			perror("Not enough memory.\n");
			fprintf(logger, "Not enough memory.\n");
			exit(1); 
		}
		dnode->docId = docId; 
		dnode->page_word_frequency = 1;
		wnode->page = dnode; 
		dnode->next = NULL; 
		strlcpy(wnode->word, word, WORD_LENGTH);  
		wnode->next = NULL;
		wnode->prev = NULL; 
		index->hash[hash_value] = wnode; 
		fprintf(logger, "word %s at hash %lu\n", word, hash_value); 

		return 1; 
	}	
	// There is a collision 
	// case 1. the word is found in hash_table. If so, then 
	// i. It was the same document -> only need to update the page_word_frequency 
	// ii. It was a diff document -> Then need to initialize that new document and set pointer 
	// case 2. there was a collision because identical hash value. If so, then 
	// i. Create a new WordNode. 
	else {
		WordNode* coll_curr = index->hash[hash_value]; 	
		while (coll_curr) {
			if (strcmp(coll_curr->word, word) == 0) {  
				if (coll_curr->page->docId == docId) {   // case 1i. 
					fprintf(logger, "collision occurred for %s at hash idx: %lu in same doc.\n", word, hash_value); 
					++coll_curr->page->page_word_frequency; 
					return 1; 	
				}
				else {
					DocNode* dnode = malloc(sizeof(DocNode)); 
					if (dnode == NULL) {
						perror("Not enough memory.\n");
						fprintf(logger, "Not enough memory.\n");
						exit(1); 
					}
					dnode->docId = docId; 
					dnode->page_word_frequency = 1; 
					DocNode* curr = coll_curr->page; // insert new DocNode at end 
					while (curr->next) {
						curr = curr->next; 
					}
					curr->next = dnode; 
					dnode->next = NULL; 

					fprintf(logger, "diff doc for word %s at hash %lu\n", word, hash_value); 
					return 1; 
				}
			}
			else if (coll_curr->next == NULL) {
				break; 
			}
			coll_curr = coll_curr->next; 
		}
		WordNode* wnode = malloc(sizeof(WordNode)); 
		if (wnode == NULL) {
			perror("Not enough memory.\n");
			fprintf(logger, "Not enough memory.\n"); 
			exit(2); 
		}
		DocNode* dnode = malloc(sizeof(DocNode)); 
		if (dnode == NULL) {
			perror("Not enough memory.\n");
			fprintf(logger, "Not enough memory.\n");
			exit(1); 
		}
		dnode->docId = docId; 
		dnode->page_word_frequency = 1;
		dnode->next = NULL;
		wnode->page = dnode; 
		strlcpy(wnode->word, word, WORD_LENGTH);  
		wnode->next = NULL;
		wnode->prev = coll_curr; 
		coll_curr->next = wnode; 

		return 1; 
	}
	return 0; 
}


void readWords(FILE *text_file, FILE* logger, int docId, INVERTED_INDEX* index) {
	char buf[WORD_LENGTH]; 
	memset(buf, 0, WORD_LENGTH);
	while (fscanf(text_file, " %s", buf) == 1) {
		char *word = malloc(strlen(buf) + 1); // strlen doesn't handle +1 null-terminator  
		if (word == NULL) {
			perror("Not enough memory.\n");
			fprintf(logger, "Not enough memory.\n"); 
			exit(1);
		}
		memcpy(word, buf, strlen(buf) + 1);  // need to copy to char* from buffer 
//		printf("word is: %s\n", word); 
		fprintf(logger, "Word is: %s\n", word); 
		if (checkWordInvalid(word)) {
			printf("word %s is invalid. Freeing memory..\n", word); 
			fprintf(logger, "word %s is invalid. Freeing memory..\n", word); 
			free(word); 
		}
		else {
			updateIndex(word, docId, index, logger); 
		}
	}
}

// driver 
int main(int argc, char** argv){
	FILE *logger = fopen("logger_index.txt", "wb"); 
	if (logger == NULL) {
		perror("file cannot be found\n");
	}
	INVERTED_INDEX* index = initInvertedIndex(logger); 
	if (argc > 1) {
		char *dir = strstr(argv[1], "url");  // need url in directory to proceed to extract
		if (dir) {
			executeExtraction(logger, dir); 
		}
		char *text_dir = strstr(argv[1], "text");  // need text to proceed normally with parsing
		if (text_dir) {
			fprintf(logger, "running parsing..\n"); 
			int nTextFiles = numFiles(text_dir); 
			char* filename = malloc(30); 
			if (filename == NULL) {
				perror("Not enough memory.\n");
				fprintf(logger, "Not enough memory.\n"); 
				exit(1);
			}	
			snprintf(filename, 30, "%s/text_%d", text_dir, 0); 
			FILE *f = fopen(filename, "rb"); 
			if (f == NULL) {
				perror("file cannot be found.\n");
				fprintf(logger, "file cannot be found.\n"); 
				exit(2);
			}
			int docId = getDocumentId(filename); 
			readWords(f, logger, docId, index); 
			free(filename); 
		}
	}
	else {
		printf("Need to pass in either directory to extract text or directory to parse\n"); 
	}
	fprintf(logger, "Now cleaning..\n"); 
	int cur = 0; 
	while (cur < MAX_HASH_SLOT) {
		if (index->hash[cur]) {
			printf("word: %s word_freq: %d\n", index->hash[cur]->word, index->hash[cur]->page->page_word_frequency);
		}
		++cur; 
	}

	for (int i =0; i < MAX_HASH_SLOT; ++i) {
		WordNode* wnode = index->hash[i]; 
		if (wnode == NULL) {
			free(wnode); 
			wnode = NULL; 
		}
		else {
			DocNode* dnode = wnode->page; 
			while (dnode) {     // free every allocated document node 
				DocNode* freedNode = dnode;
				dnode = dnode->next;   // need to make sure they are all set to NULL before freeing
				free(freedNode); 
				freedNode = NULL; 
			}
			free(wnode); 
			wnode = NULL; 
		}
	}
	free(index); 
	index = NULL;
	fprintf(logger, "Finished!\n"); 
	fclose(logger);	
	exit(0);
}
