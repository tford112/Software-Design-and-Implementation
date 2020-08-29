#ifndef _RECREATE_H_
#define _RECREATE_H_

// *****************Impementation Spec********************************

// File - recreate.c 
// This file contains the prototype for recreating the index from the index.dat 
// file that is created from running "./indexer texts" 

// MACRO 
#define BUFSIZE 1000

// PROTOTYPES 
INVERTED_INDEX* recreateIndex(FILE*, FILE*); 
#endif 
