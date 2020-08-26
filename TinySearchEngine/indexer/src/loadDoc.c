
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include "../include/loadDoc.h" 
#define SIZE 50
#define MAX_FILE_NUM 25 

void executeExtraction(FILE* log, char* dir) {
	int nfiles = numFiles(dir); 
	int count = 0; 
	fprintf(log, "Num files: %d\n", nfiles);
	char numToString[MAX_FILE_NUM];
	memset(numToString, 0, MAX_FILE_NUM); 
	fprintf(log, "running text extraction from HTMLS...\n"); 
	while (count < nfiles) {
		snprintf(numToString, MAX_FILE_NUM, "%s/%d", dir, count); // get "string" from num and pass as argument to loadDocument  
		loadDocument(numToString);  		      
		fprintf(log, "extracted file %s\n", numToString); 
		memset(numToString, 0, MAX_FILE_NUM); 		      // good practice to clear out buffer 
		++count; 
	}
}

int numFiles(char* directory) {
	struct dirent **namelist;
	int n; 
	n = scandir(directory, &namelist, NULL, alphasort);
	int num_files = 0 ; 
	if (n == -1) {
		perror("scandir could not be allocated\n");
		exit(1);
	}
	while (n--) {
		if ((strcmp(namelist[n]->d_name,".") == 0) || (strcmp(namelist[n]->d_name,"..") == 0)) {
			free(namelist[n]); 
			continue; 
		}
		++num_files;
		free(namelist[n]);
	}
	free(namelist);
	return num_files;
}


void loadDocument(char* filename) {
	char command[SIZE];
	memset(command, 0, SIZE); 
	snprintf(command, SIZE, "./extract.py %s", filename); 
	system(command);
}


