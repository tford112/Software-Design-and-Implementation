#ifndef _ALLOCATE_H_
#define _ALLOCATE_H_


// *******************Implementation Spec *******************************
//
// File: allocate.c 
// This file contains the functions for allocating memory for the index, 
// WordNodes, DocNodes, and any related containers 
#define LINE_LENGTH 1024

FILE* openFile(char*, char*); 
void checkIndexDataFile(FILE*); 
INVERTED_INDEX* allocateInvertedIndex(FILE*); 
WordNode* allocateWordNode(FILE*); 
DocNode* allocateDocNode(FILE*); 
DocNode** allocateDocNodeArray(FILE*, int); 
sharedDocId* allocateSharedId(); 
#endif 

