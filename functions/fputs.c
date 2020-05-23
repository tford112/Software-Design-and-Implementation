// writing a string to stream 
#include <stdio.h>
#include <stdlib.h>

#define SENTLEN 80 

int main() {
	FILE *fp ; 
	char sent[SENTLEN];
	printf("Enter sentence to append: ");
	fgets(sent, sizeof(sent), stdin);   // getting input in a safe way (will also null-terminate) 
	// write to file 
	fp = fopen("myfile.txt", "a"); 
	if (fp == NULL){
		perror("No file found\n"); 
		exit(1);
	}
	fputs(sent, fp); 
	fputs(sent, stdout); 
	fclose(fp);
	return 0;
}
