
#include <stdio.h>
#include "../include/and.h" 

sharedDocId* initializeSharedIds(sharedDocId*, DocNode**);
sharedDocId* filterFromSharedIds(DocNode*, sharedDocId*);
void removeSharedDocId(sharedDocId**, int);
void removeNonSharedIdsFromDocArray(DocNode**, sharedDocId*);

// we can initialize our shared Ids from our queryDocArray one time. This will be pared down later in our filterFromSharedIds 
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

// we will have to filter down our shared Ids with each query Piece 
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

