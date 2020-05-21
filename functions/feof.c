//

#include <stdio.h> 
#include <stdlib.h> 

int main() {
	FILE *fp; 
	int n = 0; 
	fp = fopen("myfile.txt", "r"); 
	if (fp == NULL) {
		perror("Error opening file\n"); 
	}
	else {
		while (fgetc(fp) != EOF) { 
			++n;
		}
		if (feof(fp)) {
			puts("End-of-file reached."); 
			printf("Total number of bytes read: %d\n", n); 
		}
	}
	fclose(fp);
	return EXIT_SUCCESS; 
}
