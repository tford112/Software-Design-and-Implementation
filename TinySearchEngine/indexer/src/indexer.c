
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "../include/loadDoc.h"
#include "../include/indexer.h" 
#include "../include/hash.h" 
#include "../include/saveClean.h" 
#include "../include/allocate.h"

/* File: indexer.c 
 * Input: Depends on usages 
 * 	Usage 1: Extract Text -> run the file as "./indexer [URL_DIRECTORY] [TARGET_DIRECTORY] 
 *      Usage 2: Parsing the Texts to create an Index File -> run the file as "./indexer [TARGET_DIRECTORY] "
 * Output: 
 * 	An index file that will have every word we extracted into our text files recorded with the total number 
	 * of documents it appears in as well as how many times it appears in each of the documents. 
 * 	For example, "administration 3 5 2 10 1 21 2" indicates that the word administration appears 3 times
 * 		2 times in document ID 5 ("text_5") 2 times, document ID 10 once, and 2 times in document ID 21 
 * Description: 
 * 	The executeExtraction() case has been addressed in loadDoc.c and extract.py. This file is concerned with 
 * 	how we can parse our text files and transform them into a .dat file as described above. We utilize a helpful
 * 	data structure, INVERTED_INDEX, which will be a hash table that will contain pointers to WordNodes. readWords()
 * 	and updateIndex() will be responsible for creating a finished index data structure that corresponds to all the 
 * 	words captured in our extracted text files. We then call "saveIndex()" which will generate the actual .dat file
 * 	in the format listed above basedon our INVERTED_INDEX index. 
 */

// driver -> can execute either executeExtraction or executeParsing. 
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

/* Given a text file, we now get all our words and format them slightly (lowercase). If the word is valid, we can update our Index*/
void readWords(FILE *text_file, FILE* logger, int docId, INVERTED_INDEX* index) {
	char buf[WORD_LENGTH] = {0}; 
	while (fscanf(text_file, " %s", buf) == 1) {
		int buf_size = WORD_LENGTH; 
		if (buf[buf_size - 1] == '.') {       			// words that end with a period would otherwise be treated as different 
			buf_size = buf_size - 1; 
		}
		for (int i = 0; i < buf_size; ++ i) {
			buf[i] = tolower(buf[i]);    
		}
		char *word = calloc((buf_size + 1), sizeof(char));
		if (word == NULL) {
			perror("Not enough memory.\n");
			fprintf(logger, "Not enough memory.\n"); 
			exit(1);
		}
		memcpy(word, buf, buf_size + 1);  			// need to copy to char* from buffer 
		if (checkWordInvalid(word)) {
			fprintf(logger, "word \"%s\" is invalid. Freeing memory..\n", word); 
		}
		else {
			updateIndex(word, docId, index, logger); 
		}
		free(word); 
	}
}

/* updateIndex() - We receive a valid word and figure out how we can update our Index. If there is no collision, then we can 
   simply just create an entry at that hash location. If there is then there are some details to work out and 
   more details can be found immediately below. Generally, there are 3 scenarios that could happen with a collision.
  	1. the collision occurred because it was because it was a different word. In this case, it was only luck that 
  	there was an identical hash-value already.
	2. the word is found in hash_table. If so, then 
	 	i. It was the same document -> only need to update the page_word_frequency 
	 		Note: We need to go through the hash list at a word collision to check whether the match exists in another doc. 
	 	ii. It was a new document -> Then need to initialize that new document and set pointer 
*/
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
	else {  
		WordNode* collCurrWord = index->hash[hash_value]; 	
		while (collCurrWord) {
			if (strcmp(collCurrWord->word, word) == 0) {  
				if (collCurrWord->page->docId == docId) {   	 // case 2 i. 
					fprintf(logger, "collision occurred for \"%s\" at hash idx: %lu in same doc.\n", word, hash_value); 
					++collCurrWord->page->page_word_frequency; 
					return 1; 	
				}
				else {   					
					DocNode* currPage = collCurrWord->page;    // case 2 ii. - Iterate through our linked list until we can make a new DocNode
					while (currPage) {
						if (currPage->docId == docId) { 
							fprintf(logger, "found doc match (%d)  within hash list for word \"%s\". Updating word freq\n", docId, word);
							++currPage->page_word_frequency; 
							return 1; 
						}
						if (currPage->next == NULL) {
							break;
						}
						currPage = currPage->next; 
					}
					DocNode* dnode = allocateDocNode(logger); 
					dnode->docId = docId; 
					dnode->page_word_frequency = 1; 
					currPage->next = dnode; 			// insert new DocNode at end  
					fprintf(logger, "diff doc (%d) for word \"%s\" at hash %lu\n", docId, word, hash_value); 
					return 1; 
				}
			}
			else if (collCurrWord->next == NULL) {
				break; 
			}
			collCurrWord = collCurrWord->next; 
		}
		DocNode* dnode = allocateDocNode(logger); 
		dnode->docId = docId; 
		dnode->page_word_frequency = 1;
		WordNode* wnode = allocateWordNode(logger); 
		wnode->page = dnode; 
		strlcpy(wnode->word, word, WORD_LENGTH);  
		wnode->prev = collCurrWord; 
		collCurrWord->next = wnode; 
		return 1; 
	}
	return 0; 
}

// get doc id from filename. We assume that the crawler saves files using progressive numbers as unique identifiers. 
int getDocumentId(char* filename) {  				// don't extract the actual "text_" part  
	char* underscore = strchr(filename, '_'); 		// get first occurrence of '_' 
	int idx = underscore - &filename[0];  			// get the index where '_' occurred
	char num[5]; 
	memcpy(num, &filename[idx+1], 4); 
	num[4] = '\0'; 
	return atoi(num); 
}

// helper function to determine if word has numbers or other garbage characters (e.g hyphen) 
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

