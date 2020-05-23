// writing a character to stream 

#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[]){
	FILE *fp;
	char c; 
	fp = fopen("myfile.txt", "a");
	if (fp != NULL) {
		for (c = 'a'; c <= 'z'; c++) {
			fputc(c, fp); // putting alphabet to file 
		}
	}
	return EXIT_SUCCESS;
}
