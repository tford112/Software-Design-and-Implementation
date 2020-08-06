
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

int readInUserInput(char*, int);
FILE* openFileContainingURL(DocNode*, char*); 
bool isWordAnd(char*); 
bool isWordOr(char*);
bool doesQueryContainAnd(char*); 
bool doesQueryContainOr(char*); 
void sendQueryPiecesToBeRead(INVERTED_INDEX*, char*, FILE*);
DocNode* searchIndexForAllDocQueryMatches(INVERTED_INDEX*, char*, FILE*);
bool checkIfDocAlreadyInArray(DocNode*, DocNode**);

void initializeQueryDocArray(DocNode*, DocNode**);
sharedDocId* initializeSharedIds(sharedDocId*, DocNode**);
sharedDocId* filterFromSharedIds(DocNode*, sharedDocId*);
void removeSharedDocId(sharedDocId**, int);
void removeNonSharedIdsFromDocArray(DocNode**, sharedDocId*);

int getNumOfDocsInArray(DocNode**);
int getNumOfSharedDocs(sharedDocId*);
void displayQueryResults(DocNode**);
int highestWordFrequency(DocNode**); 
void printCurrentQueryResult(DocNode*);

char* removeSpacesAndMakeLowerCase(char*);

/*
 *  TODO: prompt user to open a text file based on the query results 
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

// break the cleaned up query (no unnecessary whitespaces) into pieces (if query is multiple words long) to be read in filterFromSharedIds
void sendQueryPiecesToBeRead(INVERTED_INDEX* index, char* cleanQuery, FILE* log) {
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
				initializeQueryDocArray(currDoc, queryDocArray);     	  
			}
			else if (currDoc && !isWordAnd(query)) {		// case 2 -> AND detected in overall query but word itself is not 
				initializeQueryDocArray(currDoc, queryDocArray);     	  
				sdoc = initializeSharedIds(sdoc, queryDocArray); 
				sdoc = filterFromSharedIds(currDoc, sdoc);
				removeNonSharedIdsFromDocArray(queryDocArray, sdoc); 
			}
			memset(query, '\0', BUFSIZE); 
			queryPieceIter = 0; 
		}	
		++origIter; 
	}
	if (sdoc == NULL && queryContainsAnd) { 
		printf("Sorry, but no documents match complete query\n"); 
		exit(FAIL); 
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
void initializeQueryDocArray(DocNode* currDoc, DocNode** queryDocArray) {
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

// for the AND case => we can initialize our shared Ids from our queryDocArray one time. This will be pared down later in our filterFromSharedIds 
sharedDocId* initializeSharedIds(sharedDocId* sdoc, DocNode** queryDocArray) {
	int numSharedDocs = getNumOfSharedDocs(sdoc); 
	int numDocsInQueryDocArray = getNumOfDocsInArray(queryDocArray); 
	int currQueryDocIndex = 0; 
	int count = 0; 
	if (numSharedDocs == 0) {    // only initialize if there is no shared ids to begin with 
		while (currQueryDocIndex < numDocsInQueryDocArray && count < NUM_SEARCH_RESULTS) { 
			if (sdoc == NULL) {     	// linked list if head is NULL 
				sdoc = allocateSharedId(); 
				sdoc->id = queryDocArray[currQueryDocIndex]->docId; 
			}
			else {
				sharedDocId* curr = sdoc; 
				while (true) {
					if (curr->next == NULL) {
						curr->next = allocateSharedId(); 
						curr->next->id = queryDocArray[currQueryDocIndex]->docId; 
						break; 
					}
					curr = curr->next; 
				}
			}
			++currQueryDocIndex; 
		}
	}
	return sdoc; 
}

// for the AND case -> we will have to filter down our shared Ids with each query Piece 
// (e.g. computer AND science -> we gather all the docIds that are generated from searching the index for "computer", 
//  we then have to pare down our result search with "science" right after to find the intersecting results) 
sharedDocId* filterFromSharedIds(DocNode* searchResult, sharedDocId* sdoc) {
	DocNode* currDoc = searchResult; 
	bool isDocIdInShared = false; 
	sharedDocId* currSId = sdoc; 
	while (currSId != NULL)	{ 
		while (currDoc != NULL) {
			if (currSId->id == currDoc->docId) {
				isDocIdInShared = true; 
				break;
			}
			currDoc = currDoc->next; 
		}
		if (!isDocIdInShared) {
			removeSharedDocId(&sdoc, currSId->id);       // if the search result isn't found in the shared Ids, we need to remove it from shared 
			currSId = currSId->next; 
		}
		else {
			currSId = currSId->next; 
			isDocIdInShared = false; 
		}
		currDoc = searchResult;  // start process again 
	}
	return sdoc; 
}

// helper function for filterFromSharedIds to remove ids from our shared array 
void removeSharedDocId(sharedDocId** head, int docId) {
	sharedDocId* curr;
	sharedDocId* prev = NULL; 
	for (curr = *head; curr != NULL; prev = curr, curr = curr->next) {
		if (curr->id == docId) {
			if (prev == NULL) {
				*head = curr->next; 
			}
			else {
				prev->next = curr->next; 
			}
			free(curr); 
		}
	}
}

// for the AND case -> now that we have a (pared) down shared Id array, we can remove all the doc results from our queryDocArray that don't have matching 
// ids in the shared array 
void removeNonSharedIdsFromDocArray(DocNode** queryDocArray, sharedDocId* sdoc) { 
	int numDocsInQueryDocArray = getNumOfDocsInArray(queryDocArray);
	bool foundInSharedArray = false; 
	int currQueryDocIndex = 0;				        // iterator for queryDoc Array  
	sharedDocId* curr = sdoc; 
	while (currQueryDocIndex < numDocsInQueryDocArray && curr != NULL) {
		while (curr != NULL) {
			if (curr->id == queryDocArray[currQueryDocIndex]->docId) {
				foundInSharedArray = true;
				break; 
			}
			curr = curr->next;	
		}
		if (!foundInSharedArray) { 
			queryDocArray[currQueryDocIndex]->page_word_frequency = 0;  // we'll be checking later for non-zero page word freqs 
		}
		++currQueryDocIndex; 
		foundInSharedArray = false; 
		curr = sdoc; 
	}
}

// return the results of the query to screen 
void displayQueryResults(DocNode** queryDocArray) {
	int bestIndexOfDoc = -1;
	while (true) {
		bestIndexOfDoc = highestWordFrequency(queryDocArray); 
		if (bestIndexOfDoc == NUM_SEARCH_RESULTS) {
			break; 
		}
		printCurrentQueryResult(queryDocArray[bestIndexOfDoc]); 
		queryDocArray[bestIndexOfDoc]->page_word_frequency = 0; 
	}
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

int getNumOfSharedDocs(sharedDocId* sdoc) {
	int count = 0; 
	while (sdoc != NULL) {
		++count; 
		sdoc = sdoc->next; 
	}
	return count; 
}

