#ifndef _SAVECLEAN_H_
#define _SAVECLEAN_H_

// *******************Implementation Spec *******************************
//
// File: saveClean.c 
// This file contains the prototypes for saving a created index and cleaning up 
// the memory afterwards 

void saveIndex(INVERTED_INDEX*, char*, FILE*); 
void cleanUp(INVERTED_INDEX*, FILE*); 
void cleanUpWordNodesAtHashSlot(INVERTED_INDEX*, int); 
void cleanUpDocNodesFromWordNode(WordNode*); 
#endif 
