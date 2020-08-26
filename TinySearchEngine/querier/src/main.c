
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>
#include "../include/query.h"
#include "../include/allocate.h" 
#include "../include/saveClean.h" 
#include "../include/recreate.h"
#include "../include/hash.h" 
#include "../include/utils.h" 
#include "../include/and.h" 

int main(int argc, char** argv) {
	validateInputArgs(argc, argv[1]);
	FILE* recreate = openFile(argv[1], "rb");  				
	FILE* log = openFile("logger_querier.txt", "wb"); 
	INVERTED_INDEX* index = returnRecreatedIndex(recreate, log); 
	collectQueryResults(index, log); 
	cleanIndex(index, log); // clean up all allocated memory for INVERTED_INDEX 
	fclose(log); 
	fclose(recreate); 
	return 0; 
}



