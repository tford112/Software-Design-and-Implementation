
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "../include/loadDoc.h"
#include "../include/indexer.h" 
#include "../include/hash.h" 
#include "../include/saveClean.h" 
#include "../include/allocate.h"

// driver 
int main(int argc, char** argv){
	FILE *logger = openFile("logger_index.txt", "wb"); 
	INVERTED_INDEX* index = allocateInvertedIndex(logger); 
	if (argc > 1) {
		char *dir = strstr(argv[1], "url");  // need url in directory to proceed to extract
		char *text_dir = strstr(argv[1], "text");  // need text to proceed normally with parsing
		if (text_dir && dir) {
			printf("Cannot have both extract directory and parse directories in argument.\n");
			exit(1);
		}
		if (dir) {
			executeExtraction(logger, dir); 
		}
		else if (text_dir) {
			executeParsing(logger, text_dir, index); 
			printf("Done parsing..\n");
		}
		else {
			printf("Invalid directory argument needs to be directory to extract from or directory to parse texts\n"); 
			exit(2);
		}
	}
	else {
		printf("Need to pass in either directory to extract text or directory to parse\n"); 
		exit(3);
	}
	saveIndex(index, "index.dat", logger); 
	cleanUp(index, logger);
	fprintf(logger, "Finished!\n"); 
	fclose(logger);	
	exit(0);
}

void executeParsing(FILE* logger, char* text_dir, INVERTED_INDEX* index) {
	fprintf(logger, "running parsing..\n"); 
	int nTextFiles = numFiles(text_dir); 
	int file_count = 0; 
	char* filename = NULL; 
	while (file_count < nTextFiles) {
		filename = malloc(FILE_LENGTH); 
		if (filename == NULL) {
			perror("Not enough memory.\n");
			fprintf(logger, "Not enough memory.\n"); 
			exit(1);
		}	
		snprintf(filename, FILE_LENGTH, "%s/text_%d", text_dir, file_count);  // create string arg for fopen 
		FILE *f = openFile(filename, "rb"); 
		int docId = getDocumentId(filename); 
		fprintf(logger, "Reading from document: %s\n\n", filename);
		readWords(f, logger, docId, index); // start parsing this file and update index 
		free(filename); 
		fclose(f); 
		++file_count; 
	}
}

// get doc id from filename. We assume that the crawler saves files using progressive numbers as unique identifiers. 
int getDocumentId(char* filename) {  // don't extract the actual "text_" part  
	char* underscore = strchr(filename, '_'); // get first occurrence of '_' 
	int idx = underscore - &filename[0];  // get the index where '_' occurred
	char num[5]; 
	memcpy(num, &filename[idx+1], 4); 
	num[4] = '\0'; 
	return atoi(num); 
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
		DocNode* dnode = allocateDocNode(logger); 
		dnode->docId = docId; 
		dnode->page_word_frequency = 1;
		WordNode* wnode = allocateWordNode(logger); 
		wnode->page = dnode; 
		strlcpy(wnode->word, word, WORD_LENGTH);  
		index->hash[hash_value] = wnode; 
		fprintf(logger, "word \"%s\" at hash %lu for docid %d\n", word, hash_value, docId); 
		return 1; 
	}	
	// There is a collision 
	// case 1. the word is found in hash_table. If so, then 
	// i. It was the same document -> only need to update the page_word_frequency 
	// 	Note: We need to go through the hash list at a word collision to check whether the match exists in another doc. 
	// ii. It was a new document -> Then need to initialize that new document and set pointer 
	// case 2. there was a collision because identical hash value. If so, then 
	// i. Create a new WordNode. 
	else {
		WordNode* coll_curr = index->hash[hash_value]; 	
		while (coll_curr) {
			if (strcmp(coll_curr->word, word) == 0) {  
				if (coll_curr->page->docId == docId) {   	 // case 1 i. 
					fprintf(logger, "collision occurred for \"%s\" at hash idx: %lu in same doc.\n", word, hash_value); 
					++coll_curr->page->page_word_frequency; 
					return 1; 	
				}
				else {   					
					DocNode* curr_page = coll_curr->page; 
					while (curr_page) {
						if (curr_page->docId == docId) { 
							fprintf(logger, "found doc match (%d)  within hash list for word \"%s\". Updating word freq\n", docId, word);
							++curr_page->page_word_frequency; 
							return 1; 
						}
						if (curr_page->next == NULL) {
							break;
						}
						curr_page = curr_page->next; 
					}
					DocNode* dnode = allocateDocNode(logger); 
					dnode->docId = docId; 
					dnode->page_word_frequency = 1; 
					curr_page->next = dnode; 			// insert new DocNode at end  
					fprintf(logger, "diff doc (%d) for word \"%s\" at hash %lu\n", docId, word, hash_value); 
					return 1; 
				}
			}
			else if (coll_curr->next == NULL) {
				break; 
			}
			coll_curr = coll_curr->next; 
		}
		DocNode* dnode = allocateDocNode(logger); 
		dnode->docId = docId; 
		dnode->page_word_frequency = 1;
		WordNode* wnode = allocateWordNode(logger); 
		wnode->page = dnode; 
		strlcpy(wnode->word, word, WORD_LENGTH);  
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
		int buf_size = strlen(buf) ;
		if (buf[buf_size - 1] == '.') {       			// words that end with a period would otherwise be treated as different 
			buf_size = buf_size - 1; 
		}
		for (int i = 0; i < buf_size; ++ i) {
			buf[i] = tolower(buf[i]);    
		}
		char *word = malloc((buf_size + 1));
		if (word == NULL) {
			perror("Not enough memory.\n");
			fprintf(logger, "Not enough memory.\n"); 
			exit(1);
		}
		memcpy(word, buf, buf_size + 1);  			// need to copy to char* from buffer 
		if (checkWordInvalid(word)) {
			fprintf(logger, "word \"%s\" is invalid. Freeing memory..\n", word); 
			free(word); 
		}
		else {
			updateIndex(word, docId, index, logger); 
		}
	}
}


