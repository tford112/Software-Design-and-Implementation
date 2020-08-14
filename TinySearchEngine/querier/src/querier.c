
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
#include "../include/utils.h" 

void validateInputArgs(int);
int readInUserQueryInput(char*, int);
char* removeSpacesAndMakeLowerCase(char*);
void breakAndReadQuery(INVERTED_INDEX*, char*, FILE*);
DocNode* searchIndexForAllDocQueryMatches(INVERTED_INDEX*, char*, FILE*);
void updateQueryDocArray(DocNode*, DocNode**);
bool checkIfDocAlreadyInArray(DocNode*, DocNode**);

bool areThereAnyResults(DocNode**, sharedDocId*, bool);
sharedDocId* displayQueryResults(DocNode**);
int highestWordFrequency(DocNode**); 
void trackQueryIdsForUser(sharedDocId**, int);
void printCurrentQueryResult(DocNode*);

void promptUserForRequest(sharedDocId*);
bool validateUserRequest(sharedDocId*, char*);
FILE* openFileContainingURL(DocNode*, char*);

int main(int argc, char** argv) {
	validateInputArgs(argc);
	FILE* recreate = openFile(argv[1], "rb");  				
	FILE* log = openFile("logger_querier.txt", "wb"); 
	INVERTED_INDEX* index = returnRecreatedIndex(recreate, log); 
	collectQueryResults(index, log); 
	cleanIndex(index, log); 						// clean up all allocated memory for INVERTED_INDEX 
	fclose(log); 
	return 0; 
}

void validateInputArgs(int argc) {
	if (argc < 2) {
		printf("Need to provide an index to read from\n"); 
		exit(EXIT_RETURN); 
	}
	if (argc > 2) {
		printf("Too many inputs. Only need to provide the index file\n");
		exit(EXIT_RETURN); 
	}
	FILE* recreate = openFile(argv[1], "rb");  				// shell script passes in default "sorted_index.dat" -> check if empty line exists 
	checkIndexDataFile(recreate); 
	fclose(recreate); 
}

void collectQueryResults(INVERTED_INDEX* index, FILE* log) { 
	fputs("TinySearch: ", stdout); 
	char query[BUFSIZE]; 
	if (readInUserQueryInput(query, BUFSIZE)) {
		query[strlen(query)-1] = '\0';					// remove newline char from fgets  
		char* cleanQuery = removeSpacesAndMakeLowerCase(query); 
		breakAndReadQuery(index, cleanQuery, log); 
	}
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

// break the cleaned up query (no unnecessary whitespaces) into pieces (if query is multiple words long) to be read in filterFromSharedIds
void breakAndReadQuery(INVERTED_INDEX* index, char* cleanQuery, FILE* log) {
	char query[BUFSIZE]; 
	memset(query, '\0', BUFSIZE); 
	int origIter = 0, queryPieceIter = 0;  						
	bool queryContainsAnd = doesQueryContainAnd(cleanQuery); 
	bool queryContainsOr = doesQueryContainOr(cleanQuery); 
	if (queryContainsAnd && queryContainsOr) {
		printf("Cannot have both AND and OR in query. Exiting..\n"); 
		exit(FAIL); 
	}
	DocNode** queryDocArray = allocateDocNodeArray(log, NUM_SEARCH_RESULTS);  
	DocNode* currDoc = NULL; 
	sharedDocId* sdoc = NULL; 
	while (cleanQuery[origIter] != '\0') {
		if (cleanQuery[origIter] != ' ') {         			// add to buffer until reach space
			query[queryPieceIter++] = cleanQuery[origIter]; 
		}
		if (cleanQuery[origIter] == ' ' || (origIter+1 == strlen(cleanQuery))) {
			currDoc = searchIndexForAllDocQueryMatches(index, query, log); 
			if (queryContainsOr && !isWordOr(query)) {      
				updateQueryDocArray(currDoc, queryDocArray);     	  
			}
			else if (currDoc && !isWordAnd(query)) {		// case 2 -> AND detected in overall query but word itself is not 
				updateQueryDocArray(currDoc, queryDocArray);     	  
				sdoc = initializeSharedIds(sdoc, queryDocArray); 
				sdoc = filterFromSharedIds(currDoc, sdoc);
				removeNonSharedIdsFromDocArray(queryDocArray, sdoc); 
			}
			memset(query, '\0', BUFSIZE); 
			queryPieceIter = 0; 
		}	
		++origIter; 
	}
	if (areThereAnyResults(queryDocArray, sdoc, queryContainsAnd)) {
		sharedDocId* remaining = displayQueryResults(queryDocArray); 
		promptUserForRequest(remaining);
	}
	else {
		printf("Sorry, but no documents match the requested query\n"); 
	}
}

// search the Index to get the first doc that matches with the query. We will iterate through the rest of the docs from this first doc.
DocNode* searchIndexForAllDocQueryMatches(INVERTED_INDEX* index, char* queryPiece, FILE* log) {
	if (isWordAnd(queryPiece) || isWordOr(queryPiece)) {  
		return FAIL; 
	}
	unsigned hash_value = hash1(queryPiece) % MAX_HASH_SLOT; 
	WordNode* currWord = index->hash[hash_value]; 
	if (currWord == NULL) { 
		printf("query: %s does not exist in index\n", queryPiece); 
		return FAIL; 
	}
	while (currWord != NULL && strcmp(currWord->word, queryPiece) != 0) {       // hash collision -- iterate through chained list to check where's the match  
		currWord = currWord->next;  
		if (currWord == NULL) { 					   // no more words to iterate through and since no match, we exit 
			printf("query: %s does not exist in index\n", queryPiece); 
			return FAIL; 
		}
	}
	DocNode* currDoc = currWord->page; 
	return currDoc; 
}

// filling in our Query Doc Array with the results we got from searchIndexForAllDocQueryMatches 
void updateQueryDocArray(DocNode* currDoc, DocNode** queryDocArray) {
	int numDocsInQueryDocArray = getNumOfDocsInArray(queryDocArray); 
	while (currDoc) {					// iterate through all docs and check for each doc if it already exists in array
		if (!checkIfDocAlreadyInArray(currDoc, queryDocArray)) { 
			queryDocArray[numDocsInQueryDocArray++] = currDoc; 
		}
		currDoc = currDoc->next; 
	}		
}

// check if a document is already in our queryDocArray. If it is, we can add this docId to our sharedDocIntegers array 
bool checkIfDocAlreadyInArray(DocNode* currDoc, DocNode** queryDocArray) {
	int numDocsInQueryDocArray = getNumOfDocsInArray(queryDocArray); 
	int currQueryDocIndex = 0;				        // iterator for queryDoc Array  
	bool isDocAlreadyInArray = false; 
	while (currQueryDocIndex < numDocsInQueryDocArray) { 			
		if (queryDocArray[currQueryDocIndex] != NULL && queryDocArray[currQueryDocIndex]->docId == currDoc->docId) {      
			isDocAlreadyInArray = true; 
			break; 
		}
		++currQueryDocIndex; 
	}
	return isDocAlreadyInArray; 
}

bool areThereAnyResults(DocNode** queryDocArray, sharedDocId* sdoc, bool queryContainsAnd) {
	if (sdoc == NULL && queryContainsAnd) { 
		return false; 
	}
	int count = 0; 
	for (int i = 0; i < NUM_SEARCH_RESULTS; ++i) {
		if (queryDocArray[i]->page_word_frequency == 0) {
			++count; 
		}
	}
	if (count == NUM_SEARCH_RESULTS) { 
		return false; 
	}
	return true; 
}

// return the results of the query to screen 
sharedDocId* displayQueryResults(DocNode** queryDocArray) {
	int bestIndexOfDoc = -1;
	sharedDocId* remaining = NULL;
	while (true) {
		bestIndexOfDoc = highestWordFrequency(queryDocArray); 
		if (bestIndexOfDoc == NUM_SEARCH_RESULTS) {
			break; 
		}
		trackQueryIdsForUser(&remaining, queryDocArray[bestIndexOfDoc]->docId); 
		printCurrentQueryResult(queryDocArray[bestIndexOfDoc]); 
		queryDocArray[bestIndexOfDoc]->page_word_frequency = 0; 
	}
	return remaining; 
}

// keep track of the ids for the user to select a doc from 
void trackQueryIdsForUser(sharedDocId** remaining, int bestIndexOfDoc) {
	if (*remaining == NULL) {
		*remaining = allocateSharedId(); 
		(*remaining)->id = bestIndexOfDoc; 
	}
	else {
		sharedDocId* curr = *remaining; 
		sharedDocId* nextNode = NULL; 
		nextNode = allocateSharedId(); 
		nextNode->id = bestIndexOfDoc;
		while (curr->next != NULL) {
			curr = curr->next; 
		}	
		curr->next = nextNode; 
	}
}

// helper function for displayQueryResults -> here we display our current doc query result to user 
void printCurrentQueryResult(DocNode* currBestDocNode) {
	char url[BUFSIZE]; 
	memset(url, '\0', BUFSIZE); 
	FILE* url_file; 
	url_file = openFileContainingURL(currBestDocNode, url);
	fgets(url, BUFSIZE, url_file); 			     	     // extract the first line which contains the URL 
	printf("Document ID: %d with word frequency: %d URL: %s", currBestDocNode->docId, currBestDocNode->page_word_frequency, url); 
	fclose(url_file); 
}

void promptUserForRequest(sharedDocId* remaining) {
	char buf[BUFSIZE]; 
	memset(buf, 0, BUFSIZE); 
	while (true) {
		printf("\nPlease choose a Doc Id result to open: \n"); 
		fgets(buf, BUFSIZE, stdin); 
		buf[strlen(buf)-1] = '\0';
		if (validateUserRequest(remaining, buf)) {
			printf("Selecting doc: %s\n", buf); 
			char request[BUFSIZE] = {0}; 
			snprintf(request, BUFSIZE, "texts/text_%s", buf); 
			FILE* userRequestFile = openFile(request, "r");
			char c; 
			do {
				c = fgetc(userRequestFile); 	
				printf("%c", c); 
			} while (c != EOF); 
			fclose(userRequestFile); 		
			break; 
		}
		memset(buf, 0, BUFSIZE); 
	}
}

bool validateUserRequest(sharedDocId* remaining, char* request) {
	if (!atoi(request)) {
		printf("Please provide a number representing Doc Id to open the file\n"); 
		return false; 
	}
	sharedDocId* curr = remaining; 
	while (curr != NULL) {
		if (curr->id == atoi(request)) {
			return true; 
		}
		curr = curr->next; 
	}
	printf("Sorry, but your doc Id is not in the query results\n"); 
	return false; 
}

// simple ranking algorithm to output the doc node with the highest page word frequency. 
int highestWordFrequency(DocNode** queryDocArray) {
	int max = -1;
	int max_index = 0;
	int emptyCount = 0; 
	for (int i = 0; i < NUM_SEARCH_RESULTS; ++i) {
		if (queryDocArray[i] == NULL || queryDocArray[i]->page_word_frequency == 0) {
			++emptyCount; 
			continue;
		}
		if (queryDocArray[i]->page_word_frequency > max) {
			max = queryDocArray[i]->page_word_frequency; 
			max_index = i; 
		}
	}
	if (emptyCount == NUM_SEARCH_RESULTS) {    // will occur once there are no more results 
		return emptyCount; 
	}
	return max_index;  
}


// helper function for displayQueryResults to return the file that has the URL from queried page 
FILE* openFileContainingURL(DocNode* currDoc, char* url) {
	FILE* url_file; 
	url_file = openFile(url, "rb"); 
	return url_file; 
}

