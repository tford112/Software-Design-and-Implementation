/* fread.c is necessary for reading in files where we don't know their size ahead of time 
 * seek all the way to EOF, save the location, and then rewind to beginning 
 * Now can malloc/initialize buffer to read in the appropriate # of bytes from the file 
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main() {
	FILE *fp; 
	long lSize;
	char *buf;
	size_t result; 

	fp = fopen("myfile.txt", "r"); 
	if (fp == NULL) {
		fputs("file error\n", stderr);
		exit(1);
	}

	fseek(fp, 0, SEEK_END); // seek to end of file 
	lSize = ftell(fp); 	// get current fp 
	rewind(fp);		// rewind to beginning of file 

	buf = malloc(sizeof(char) * lSize + 1); // allocate buffer based on number of bytes +1 for '\0'
	if (buf == NULL){ 
		perror("Not enough memory\n");
		exit(2);
	}
	memset(buf, 0, lSize +1); //initialize malloc'ed buffer 
	result = fread(buf, sizeof(char), lSize, fp);  // (buffer, element size, total # of bytes, file) 
	if (result != lSize){
		fputs("reading error", stderr);
		exit(3);		// different exit codes for different errors 
	}
	fputs(buf, stdout); 
	fclose(fp);
	free(buf);
	return EXIT_SUCCESS;
}

