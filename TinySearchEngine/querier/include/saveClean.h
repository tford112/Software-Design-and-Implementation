#ifndef _SAVECLEAN_H_
#define _SAVECLEAN_H_

// *******************Implementation Spec *******************************
//
// File: saveClean.c 
// This file contains the prototypes for saving a created index and cleaning up 
// the memory afterwards 
#include "nums.h" 
void saveIndex(INVERTED_INDEX*, char*, FILE*); 
void cleanIndex(INVERTED_INDEX*, FILE*); 
#endif 
