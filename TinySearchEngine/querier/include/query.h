
#ifndef _QUERY_H_
#define _QUERY_H_

// *****************Impementation Spec********************************
// File: query.c
// This file contains useful information for implementing query:
// - DEFINES
// - MACROS
// - DATA STRUCTURES
// - PROTOTYPES

#include "nums.h" 
#include <stdbool.h>

// DATA STRUCTURES. All these structures should be malloc 'd

// This is the key data structure that holds the information of each URL.

typedef struct _DocumentNode {
	struct _DocumentNode *next;  // pointer to next member in list 
	int docId;		     // doc identifier  
	int page_word_frequency;     // # of occurrences in word  
} _DocumentNode; 

typedef struct _DocumentNode DocNode; // alias 

typedef struct _WordNode {
	struct _WordNode *prev;       // pointer to prev word 
	struct _WordNode *next;	      // pointer to next word  
	char word[WORD_LENGTH];       
	DocNode *page;		      // pointer to first element in page list 
} _WordNode; 

typedef struct _WordNode WordNode; 

WordNode *WordList; 

typedef struct _INVERTED_INDEX {
	WordNode *hash[MAX_HASH_SLOT];  // hash slot 
} _INVERTED_INDEX;

typedef struct _INVERTED_INDEX INVERTED_INDEX; 

typedef struct _SharedDocId {
	struct _SharedDocId *next;
	int id; 
} _SharedDocId; 

typedef struct _SharedDocId sharedDocId; 

//PROTOTYPES
// we get the user query and break it up into multiple pieces. Each piece will be cross-checked 
// with our index. If it exists, we track the document nodes it appears in via id. We create a 
// search query array storing all these results. If it is the OR case, then for the next query piece
// we only have to avoid duplicates. If it is the AND case (e.g. "computer AND science" which is also
// "computer science"), then we have several functions in the "and.c" file that will handle those. 
void collectQueryResults(INVERTED_INDEX*, FILE*); 
void breakAndReadQuery(INVERTED_INDEX*, char*, FILE*);
DocNode* searchIndexForAllDocQueryMatches(INVERTED_INDEX*, char*, FILE*);
void updateQueryDocArray(DocNode*, DocNode**);
bool checkIfDocAlreadyInArray(DocNode*, DocNode**);

// after computing what results (or in the case of AND, what shared results) appear,
// we rank the search results with a very simple ranking algorithm (highestWordFrequency) 
// and display the results in ranked order. 
bool areThereAnyResults(DocNode**, sharedDocId*, bool);
sharedDocId* displayQueryResults(DocNode**);
int highestWordFrequency(DocNode**); 
void trackQueryIdsForUser(sharedDocId**, int);
void printCurrentQueryResult(DocNode*);

// after getting the queries and displaying them, we prompt the user to make 
// a doc id selection so as to open that file and read it 
void promptUserForRequest(sharedDocId*);
bool validateUserRequest(sharedDocId*, char*);

#endif

