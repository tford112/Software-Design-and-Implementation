
#ifndef _AND_H_
#define _AND_H_

// *******************Implementation Spec *******************************
//
// File: and.c 
// This file contains the prototypes for the AND case which will be the most 
// common case for our query search engine. Because we have to have a collection 
// of shared documents for each query word that needs to be filtered down, it's 
// best to have all the related functions in a separate file. 
#include <stdlib.h> 
#include "nums.h" 
#include "query.h" 
#include "utils.h" 
#include "allocate.h" 
sharedDocId* initializeSharedIds(sharedDocId*, DocNode*);
sharedDocId* filterFromSharedIds(DocNode*, sharedDocId*);
void removeSharedDocId(sharedDocId**, int);
void removeNonSharedIdsFromDocArray(DocNode*, sharedDocId*);


#endif 
