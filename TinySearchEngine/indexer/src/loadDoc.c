
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include "../include/loadDoc.h" 
#define SIZE 50
#define MAX_FILE_NUM 25 

/* File: loadDoc.c 
 * Input -> directory where our text files are located 
 * Output -> the located directory will now have text versions of the urls located in "urls" and was passed from crawler 
 *
 * Description -> I decided to directly use Python because I'm the most familiar with the BeautifulSoup extraction and wanted to reduce 
 * the complexity (C doesn't appear to have as good of a HTML parser as Python). This file is mostly involved with being a helper function 
 * via C to execute the extract.py file in the directory. 
 */ 

/*Responsible for executing the extract.py function. Calls the numFiles() function to figure out how many files we will need to extract
 * We directly call from the command-line with loadDocument to extract the text from a url file*/
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
	char command[SIZE] = {0};
	snprintf(command, SIZE, "./extract.py %s", filename); 
	system(command);
}


