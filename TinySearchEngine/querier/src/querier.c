
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>
#include "../include/querier.h"
#include "../include/allocate.h" 
#include "../include/saveClean.h" 
#include "../include/recreate.h"
#include "../include/hash.h" 

#define NUM_SEARCH_RESULTS 100 

int readInUserInput(char*, int);
FILE* openFileContainingURL(DocNode*, char*); 
bool isWordAnd(char*); 
bool isWordOr(char*);
bool doesQueryContainAnd(char*); 
bool doesQueryContainOr(char*); 
void sendQueryPiecesToBeRead(INVERTED_INDEX*, char*, FILE*);
DocNode* searchIndexForAllDocQueryMatches(INVERTED_INDEX*, char*, FILE*);
void filterQueryResults(DocNode*, DocNode**, int*, bool, bool);
int getNumOfDocsInArray(DocNode**);
int getNumOfSharedDocs(int*);
bool checkIfDocAlreadyInArray(DocNode*, DocNode**, int*, bool);

void displayQueryResults(DocNode**);
char* removeSpacesAndMakeLowerCase(char*);


 /* TODO: rank results based on how many occurrences of each word appear in each document 
 */
int main(int argc, char** argv) {
	if (argc < 2) {
		printf("Need to provide an index to read from\n"); 
		exit(EXIT_RETURN); 
	}
	if (argc > 2) {
		printf("Too many inputs. Only need to provide the index file\n");
		exit(EXIT_RETURN); 
	}
	FILE* recreate = openFile(argv[1], "rb");  			// shell script will pass in default "sorted_index.dat" 
	checkIndexDataFile(recreate); 
	FILE* log = openFile("logger_querier.txt", "wb"); 
	INVERTED_INDEX* index = returnRecreatedIndex(recreate, log); 
	char query[BUFSIZE]; 
	fputs("TinySearch: ", stdout); 
	if (readInUserInput(query, BUFSIZE)) {
		query[strlen(query)-1] = '\0';				// remove newline char from fgets  
		char* cleanQuery = removeSpacesAndMakeLowerCase(query); 
		sendQueryPiecesToBeRead(index, cleanQuery, log); 
	}
	cleanIndex(index, log); 						// clean up all allocated memory for INVERTED_INDEX 
	fclose(log); 
	return 0; 
}

// helper function for displayQueryResults to return the file that has the URL from queried page 
FILE* openFileContainingURL(DocNode* currDoc, char* url) {
	FILE* url_file; 
	snprintf(url, BUFSIZE, "urls/%d", currDoc->docId);   	     // the original URL text file has the url 
	url_file = openFile(url, "rb"); 
	return url_file; 
}

// get the user input 
int readInUserInput(char* buf, int size) {
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

// break the cleaned up query (no unnecessary whitespaces) into pieces (if query is multiple words long) to be read in filterQueryResults
void sendQueryPiecesToBeRead(INVERTED_INDEX* index, char* cleanQuery, FILE* log) {
	char query[BUFSIZE]; 
	memset(query, '\0', BUFSIZE); 
	int origIter = 0, queryPieceIter = 0;  						
	bool queryContainsAnd = doesQueryContainAnd(cleanQuery); 
	bool queryContainsOr = doesQueryContainOr(cleanQuery); 
	DocNode** queryDocArray = allocateDocNodeArray(log, NUM_SEARCH_RESULTS);  
	DocNode* currDoc; 
	int* sharedDocIntegers = allocateIntArray(NUM_SEARCH_RESULTS); // store into int array for AND case to filter only for these 
	while (cleanQuery[origIter] != '\0') {
		if (cleanQuery[origIter] != ' ') {         			// add to buffer until reach space
			query[queryPieceIter++] = cleanQuery[origIter]; 
		}
		if (cleanQuery[origIter] == ' ' || (origIter+1 == strlen(cleanQuery))) {
			currDoc = searchIndexForAllDocQueryMatches(index, query, log); 
			if (currDoc && !queryContainsAnd && !queryContainsOr) {  	// case 1 -> no AND or OR in query overall 
				printf("case1 sending...%s\n", query); 
				filterQueryResults(currDoc, queryDocArray, sharedDocIntegers, false, false); 
			}
			else if (currDoc && queryContainsAnd && !isWordAnd(query)) {	// case 2 -> AND detected in overall query but word itself is not 
				printf("case2\n");
				filterQueryResults(currDoc, queryDocArray, sharedDocIntegers, true, false); 
			}
			else if (currDoc && queryContainsOr && !isWordOr(query)) {      // case 3 -> OR detected in overall query but word itself is not
				filterQueryResults(currDoc, queryDocArray, sharedDocIntegers, false, true); 
			}
			memset(query, '\0', BUFSIZE); 
			queryPieceIter = 0; 
		}	
		++origIter; 
	}
	displayQueryResults(queryDocArray); 
}

// search the Index to get the first doc that matches with the query. We will iterate through the rest of the docs from this first doc.
DocNode* searchIndexForAllDocQueryMatches(INVERTED_INDEX* index, char* queryPiece, FILE* log) {
	if (isWordAnd(queryPiece) || isWordOr(queryPiece)) {  
		return FAIL; 
	}
	unsigned hash_value = hash1(queryPiece) % MAX_HASH_SLOT; 
	WordNode* currWord = index->hash[hash_value]; 
	if (!currWord) { 
		printf("query: %s does not exist in index\n", queryPiece); 
		return FAIL; 
	}
	while (strcmp(currWord->word, queryPiece) != 0) {   	     // hash collision -- iterate through chained list to check where's the match  
		currWord = currWord->next;  
	}
	DocNode* currDoc = currWord->page; 
	return currDoc; 
}

// Filter all the DocNode results based on whether AND or OR was detected in query 
void filterQueryResults(DocNode* currDoc, DocNode** queryDocArray, int* sharedDocIntegers, bool andDetected, bool orDetected) { 
	int numDocsInQueryDocArray = getNumOfDocsInArray(queryDocArray); 

	bool isDocAlreadyInArray = false;  				
	printf("currdoc before %d\n", currDoc->docId); 
	while (currDoc) {					// iterate through all docs and check for each doc if it already exists in array
		isDocAlreadyInArray = checkIfDocAlreadyInArray(currDoc, queryDocArray, sharedDocIntegers, andDetected); 
		if (!isDocAlreadyInArray) {       
			queryDocArray[numDocsInQueryDocArray++] = currDoc; 
		}
		currDoc = currDoc->next; 
	}		
	if (andDetected) {
		int currSharedDocIndex = 0; 		// iterator for shared integer array containing doc nums that already exist in array
		bool foundInSharedArray = false; 
		int currQueryDocIndex = 0;				        // iterator for queryDoc Array  
		int numSharedDocs = getNumOfSharedDocs(sharedDocIntegers); 
		while (currQueryDocIndex < numDocsInQueryDocArray && numSharedDocs > 0) {
			while (currSharedDocIndex < numSharedDocs) {
				if (sharedDocIntegers[currSharedDocIndex] == queryDocArray[currQueryDocIndex]->docId) {
					foundInSharedArray = true;
					break; 
				}
				++currSharedDocIndex; 
			}
			if (!foundInSharedArray) { 
				queryDocArray[currQueryDocIndex]->page_word_frequency = 0;  // we'll be checking later for non-blank page word freqs 
			}
			++currQueryDocIndex; 
			currSharedDocIndex = 0; 
			foundInSharedArray = false; 
		}
	}
}

bool checkIfDocAlreadyInArray(DocNode* currDoc, DocNode** queryDocArray, int* sharedDocIntegers, bool andDetected) {
	int numDocsInQueryDocArray = getNumOfDocsInArray(queryDocArray); 
	int numSharedDocs = getNumOfSharedDocs(sharedDocIntegers); 
	int currQueryDocIndex = 0;				        // iterator for queryDoc Array  
	bool isDocAlreadyInArray = false; 
	while (currQueryDocIndex < numDocsInQueryDocArray) { 			
		if (queryDocArray[currQueryDocIndex] != NULL && queryDocArray[currQueryDocIndex]->docId == currDoc->docId) {      
			isDocAlreadyInArray = true; 
			if (andDetected) {
				sharedDocIntegers[numSharedDocs] = currDoc->docId;   
				//printf("Found doc match already in array: %d\n", sharedDocIntegers[numSharedDocs]); 
			}
			break; 
		}
		++currQueryDocIndex; 
	}
	return isDocAlreadyInArray; 
}

// return the results of the query to screen 
void displayQueryResults(DocNode** queryDocArray) {
	char url[BUFSIZE]; 
	FILE* url_file; 
	for (int i = 0; i < NUM_SEARCH_RESULTS; ++i) { 	
		if (queryDocArray[i] == NULL || queryDocArray[i]->page_word_frequency == 0) {
			continue;
		}
		url_file = openFileContainingURL(queryDocArray[i], url);
		memset(url, '\0', BUFSIZE); 
		fgets(url, BUFSIZE, url_file); 			     	     // extract the first line which contains the URL 
		printf("Document ID: %d URL: %s", queryDocArray[i]->docId, url); 
		fclose(url_file); 
		memset(url, '\0', BUFSIZE); 
	}
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
	while (queryDocArray[numDocsInQueryDocArray]->page_word_frequency != 0) { // iterate through array until it's empty to update there 
		++numDocsInQueryDocArray; 
	}
	return numDocsInQueryDocArray; 
}

int getNumOfSharedDocs(int* sharedDocs) {
	int count = 0; 
	if (sharedDocs[0] == 0 && sharedDocs[1] == 0) {     // shared docs is calloc'ed but there is a doc w/ id 0 so check id0 and id1  
		return count;
	}
	for (int i = 0; i < NUM_SEARCH_RESULTS; ++i) {
		if (sharedDocs[i] == 0 && sharedDocs[i+1] != NUM_SEARCH_RESULTS && sharedDocs[i+1] == 0) {
			break; 
		}
		++count; 
	}
	return count; 
}

