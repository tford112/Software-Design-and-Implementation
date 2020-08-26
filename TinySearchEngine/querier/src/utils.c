
#include <stdio.h>
#include <stdlib.h>
#include "../include/utils.h" 

// first check whether the correct arguments (number and existence) for running file 
void validateInputArgs(int argc, char* indexToSearch) {
	if (argc < 2) {
		printf("Need to provide an index to read from\n"); 
		exit(EXIT_RETURN); 
	}
	if (argc > 2) {
		printf("Too many inputs. Only need to provide the index file\n");
		exit(EXIT_RETURN); 
	}
	FILE* recreate = openFile(indexToSearch, "rb");  // .sh script passes in "sorted_index.dat" -> check if empty line exists 
	checkIndexDataFile(recreate); 
	fclose(recreate); 
}

// get the user input 
int readInUserQueryInput(char* buf, int size) {
	if (fgets(buf, size, stdin)) {
		return feof(stdin) || (strlen(buf) != 0); 
	}
	return FAIL;  
}

// remove any leading and trailing whitespaces that a user may include as well as turning query into lowercase 
char* removeSpacesAndMakeLowerCase(char* query) {
	char queryNoWhiteSpaces[BUFSIZE]; 
	memset(queryNoWhiteSpaces, '\0', BUFSIZE); 
	int newQueryCount = 0, i = 0; 
	while (query[i] != '\0') {
		if (query[i] == ' ' && (i+1 < strlen(query) && query[i+1] != ' ' && i-1 > 0 && query[i-1] != ' ')) { // only acceptable whitespace is the one between 2 characters 
			queryNoWhiteSpaces[newQueryCount++] = ' '; 
		}
		else if (isalpha(query[i])) {
			queryNoWhiteSpaces[newQueryCount++] = tolower(query[i]); 
		}
		++i;
	}
	char* cleanQuery = malloc(BUFSIZE); 
	if (cleanQuery == NULL) {
		perror("Not enough memory\n");
		exit(FAIL); 
	}
	strlcpy(cleanQuery, queryNoWhiteSpaces, BUFSIZE); 
	printf("Query: %s\n", cleanQuery); 
	return cleanQuery; 
}

bool doesQueryContainAnd(char* cleanQuery) {
	if (strstr(cleanQuery, "and") != NULL) {
		return true; 
	}
	return false; 
}

bool doesQueryContainOr(char* cleanQuery) {
	if (strstr(cleanQuery, "or") != NULL) {
		return true; 
	}
	return false; 
}

bool isWordAnd(char* queryPiece) {
	if (strcmp(queryPiece, "and") == 0) {
		return true;
	}
	return false; 
}

bool isWordOr(char* queryPiece) {
	if (strcmp(queryPiece, "or") == 0) {
		return true;
	}
	return false; 
}

int getNumOfDocsInArray(DocNode* queryDocArray) {
	int numDocsInQueryDocArray = 0; 
	while (queryDocArray[numDocsInQueryDocArray].page_word_frequency != 0) {   // iterate through array until empty for an update 
		++numDocsInQueryDocArray; 
	}
	return numDocsInQueryDocArray; 
}

int getNumOfSharedDocs(sharedDocId* sdoc) {
	int count = 0; 
	while (sdoc != NULL) {
		++count; 
		sdoc = sdoc->next; 
	}
	return count; 
}

// helper function for collectAndDisplayQueryResults to return the file that has the URL from queried page 
FILE* openFileContainingURL(DocNode* currDoc, char* url) {
	FILE* url_file; 
	snprintf(url, BUFSIZE, "../src/urls/%d", currDoc->docId);    // the original URL text file has the url 
	url_file = openFile(url, "rb"); 
	return url_file; 
}

