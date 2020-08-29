#ifndef _LOADDOC_H_
#define _LOADDOC_H_

// *****************Impementation Spec********************************
// File: loadDoc.c
// This file contains useful information for the loadDoc file that will be responsible for converting 
// the urls we got from the crawler into workable text files (texts that will be used for the querier) 


// - PROTOTYPES
int numFiles();
void loadDocument(char*); 
void executeExtraction(FILE* log, char* dir); 
#endif 
