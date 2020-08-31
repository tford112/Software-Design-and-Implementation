
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>
#include "../include/query.h" 
#include "../include/allocate.h" 
#include "../include/clean.h" 
#include "../include/recreate.h"
#include "../include/hash.h" 
#include "../include/utils.h" 
#include "../include/and.h" 

/* FILE: query.c 
 * DESCRIPTION: we get the user query and break it up into multiple pieces. Each piece will be cross-checked 
	with our index. If it exists, we track the document nodes it appears in via id. We create a 
	search query array storing all these results. If it is the OR case, then for the next query piece
	we only have to avoid duplicates. If it is the AND case (e.g. "computer AND science" which is also
	"computer science"), then we have several functions in the "and.c" file that will handle those. 
	after computing what results (or in the case of AND, what shared results) appear,
	we rank the search results with a very simple ranking algorithm (highestWordFrequency) 
	and display the results in ranked order. After getting the queries and displaying them, we prompt 
	the user to make a doc id selection so as to open that file and read it  
*/


void collectQueryResults(INVERTED_INDEX* index, FILE* log) { 
	fputs("TinySearch: ", stdout); 
	char query[BUFSIZE]; 
	if (readInUserQueryInput(query, BUFSIZE)) {
		query[strlen(query)-1] = '\0';					// remove newline char from fgets  
		char* cleanQuery = removeSpacesAndMakeLowerCase(query); 
		breakAndReadQuery(index, cleanQuery, log); 
		free(cleanQuery); 
	}
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
	DocNode* queryDocArray = allocateDocNodeArray(NUM_SEARCH_RESULTS);  
	DocNode* currDoc = NULL; 
	sharedDocId* sdoc = NULL; 
	sharedDocId* remaining = NULL; 
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
		remaining = displayQueryResults(queryDocArray); 
		promptUserForRequest(remaining);
	}
	else {
		printf("Sorry, but no documents match the requested query\n"); 
	}
	free(queryDocArray); 
	fputs("Cleaned queryDocArray..", log);
	cleanSharedIds(sdoc); 
	cleanSharedIds(remaining);
	fputs("Cleaned sharedDocIds..", log); 
}

// search the Index to get the first doc that matches with the query. We will iterate through the rest of the docs from this first doc.
DocNode* searchIndexForAllDocQueryMatches(INVERTED_INDEX* index, char* queryPiece, FILE* log) {
	if (isWordAnd(queryPiece) || isWordOr(queryPiece)) {  
		return FAIL; 
	}
	unsigned hash_value = hash1(queryPiece) % MAX_HASH_SLOT; 
	WordNode* currWord = index->hash[hash_value]; 
	if (currWord == NULL) { 
		printf("%s does not exist in index\n", queryPiece); 
		return FAIL; 
	}
	while (currWord != NULL && strcmp(currWord->word, queryPiece) != 0) {       // hash collision -- iterate through chained list to check where's the match  
		currWord = currWord->next;  
		if (currWord == NULL) { 					   // no more words to iterate through and since no match, we exit 
			printf("%s does not exist in index\n", queryPiece); 
			return FAIL; 
		}
	}
	DocNode* currDoc = currWord->page; 
	return currDoc; 
}

// filling in our Query Doc Array with the results we got from searchIndexForAllDocQueryMatches 
void updateQueryDocArray(DocNode* currDoc, DocNode* queryDocArray) {
	int numDocsInQueryDocArray = getNumOfDocsInArray(queryDocArray); 
	while (currDoc) {		// iterate through all docs and check for each doc if it already exists in array
		if (!checkIfDocAlreadyInArray(currDoc, queryDocArray)) { 
			queryDocArray[numDocsInQueryDocArray].docId = currDoc->docId; 
			queryDocArray[numDocsInQueryDocArray].page_word_frequency = currDoc->page_word_frequency; 
			queryDocArray[numDocsInQueryDocArray].next = currDoc->next; 
			numDocsInQueryDocArray++; 
		}
		currDoc = currDoc->next; 
	}		
}

// check if a document is already in our queryDocArray. If it is, we can add this docId to our sharedDocIntegers array 
bool checkIfDocAlreadyInArray(DocNode* currDoc, DocNode* queryDocArray) {
	int numDocsInQueryDocArray = getNumOfDocsInArray(queryDocArray); 
	int currQueryDocIndex = 0;				        // iterator for queryDoc Array  
	bool isDocAlreadyInArray = false; 
	while (currQueryDocIndex < numDocsInQueryDocArray) { 			
		if (queryDocArray[currQueryDocIndex].docId == currDoc->docId) {      
			isDocAlreadyInArray = true; 
			break; 
		}
		++currQueryDocIndex; 
	}
	return isDocAlreadyInArray; 
}

bool areThereAnyResults(DocNode* queryDocArray, sharedDocId* sdoc, bool queryContainsAnd) {
	if (sdoc == NULL && queryContainsAnd) { 
		return false; 
	}
	int count = 0; 
	for (int i = 0; i < NUM_SEARCH_RESULTS; ++i) {
		if (queryDocArray[i].page_word_frequency == 0) {
			++count; 
		}
	}
	if (count == NUM_SEARCH_RESULTS) { 
		return false; 
	}
	return true; 
}

// return the results of the query to screen 
sharedDocId* displayQueryResults(DocNode* queryDocArray) {
	int bestIndexOfDoc = -1;
	sharedDocId* remaining = NULL;
	while (true) {
		bestIndexOfDoc = highestWordFrequency(queryDocArray); 
		if (bestIndexOfDoc == NUM_SEARCH_RESULTS) {
			break; 
		}
		trackQueryIdsForUser(&remaining, queryDocArray[bestIndexOfDoc].docId); 
		printCurrentQueryResult(&queryDocArray[bestIndexOfDoc]); 
		queryDocArray[bestIndexOfDoc].page_word_frequency = 0; 
	}
	return remaining; 
}

// simple ranking algorithm to output the doc node with the highest page word frequency. 
int highestWordFrequency(DocNode* queryDocArray) {
	int max = -1;
	int max_index = 0;
	int emptyCount = 0; 
	for (int i = 0; i < NUM_SEARCH_RESULTS; ++i) {
		if (queryDocArray[i].page_word_frequency == 0) {
			++emptyCount; 
			continue;
		}
		if (queryDocArray[i].page_word_frequency > max) {
			max = queryDocArray[i].page_word_frequency; 
			max_index = i; 
		}
	}
	if (emptyCount == NUM_SEARCH_RESULTS) {    // will occur once there are no more results 
		return emptyCount; 
	}
	return max_index;  
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
	char url[BUFSIZE] = {0}; 
	FILE* url_file = openFileContainingURL(currBestDocNode, url);
	fgets(url, BUFSIZE, url_file); 	    // extract the first line which contains the URL 
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
			snprintf(request, BUFSIZE, "../src/texts/text_%s", buf); 
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


