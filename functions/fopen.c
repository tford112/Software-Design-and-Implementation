
// source: cplusplus.com 

#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[]){
	FILE *fp; 
	fp = fopen("myfile.txt", "a");
	if (fp != NULL) {
		fputs("-fopen example-", fp);
		fclose(fp);
	}
	else {
		perror("Error opening file\n");
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}
