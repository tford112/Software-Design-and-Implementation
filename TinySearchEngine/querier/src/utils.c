
#ifndef STDIO_H_
#define STDIO_H_
#include <stdio.h>
#endif 

int readInUserQueryInput(char*, int);
FILE* openFileContainingURL(DocNode*, char*); 
bool isWordAnd(char*); 
bool isWordOr(char*);
bool doesQueryContainAnd(char*); 
bool doesQueryContainOr(char*); 
int getNumOfDocsInArray(DocNode**);
int getNumOfSharedDocs(sharedDocId*);

// helper function for collectAndDisplayQueryResults to return the file that has the URL from queried page 
FILE* openFileContainingURL(DocNode* currDoc, char* url) {
	FILE* url_file; 
	snprintf(url, BUFSIZE, "urls/%d", currDoc->docId);   	     // the original URL text file has the url 
	url_file = openFile(url, "rb"); 
	return url_file; 
}

// get the user input 
int readInUserQueryInput(char* buf, int size) {
	if (fgets(buf, size, stdin)) {
		return feof(stdin) || (strlen(buf) != 0); 
	}
	return FAIL;  
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

int getNumOfDocsInArray(DocNode** queryDocArray) {
	int numDocsInQueryDocArray = 0; 
	while (queryDocArray[numDocsInQueryDocArray]->page_word_frequency != 0) {   // iterate through array until empty for an update 
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

