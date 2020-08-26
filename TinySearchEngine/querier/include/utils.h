
#ifndef _UTILS_H_
#define _UTILS_H_

// *******************Implementation Spec *******************************
//
// File: utils.c 
// This file contains the prototypes for the UTILS case which will have our 
// helper functions. 
// reading in User Input -> validateInputArgs, readInUserQueryInput and removeSpacesAndMakeLowerCase 
// checking whether query piece has an AND or OR -> isWordAnd, isWordOr, doesQueryContainAnd, doesQueryContainOr
// "getters" -> getNumOfDocsInArray, getNumOfSharedDocs 
//
#include <stdbool.h> 
#include <string.h> 
#include <ctype.h> 
#include "nums.h"
#include "allocate.h" 
#include "recreate.h"
#include "query.h" 
void validateInputArgs(int, char*);
int readInUserQueryInput(char*, int);
char* removeSpacesAndMakeLowerCase(char* query);
FILE* openFileContainingURL(DocNode*, char*); 
bool isWordAnd(char*); 
bool isWordOr(char*);
bool doesQueryContainAnd(char*); 
bool doesQueryContainOr(char*); 
int getNumOfDocsInArray(DocNode*);
int getNumOfSharedDocs(sharedDocId*);

#endif 
