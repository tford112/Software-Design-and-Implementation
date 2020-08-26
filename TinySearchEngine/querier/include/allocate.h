#ifndef _ALLOCATE_H_
#define _ALLOCATE_H_


// *******************Implementation Spec *******************************
//
// File: allocate.c 
// This file contains the functions for allocating memory for the index, 
// WordNodes, DocNodes, and any related containers 
#define LINE_LENGTH 1024
#include "nums.h"
#include "query.h" 
FILE* openFile(char*, char*); 
void checkIndexDataFile(FILE*); 
INVERTED_INDEX* allocateInvertedIndex(FILE*); 
WordNode* allocateWordNode(FILE*); 
//DocNode* allocateDocNode(FILE*); 
DocNode* allocateDocNodeArray(int); 
sharedDocId* allocateSharedId(); 
#endif 

