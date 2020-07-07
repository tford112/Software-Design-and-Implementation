
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#define SIZE 50

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
		if ((strcmp(namelist[n]->d_name,".") == 0) || (strcmp(namelist[n]->d_name,"..") == 0)) continue; 
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


