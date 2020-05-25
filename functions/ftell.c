
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[]){
	FILE *fp = fopen("myfile.txt", "r"); 
	if (fp == NULL){
		perror("No file found\n");
		exit(1);
	}
	fseek(fp, 0, SEEK_END) ;  // seek to the end of the file and then store # of bytes 
	long size = ftell(fp);
	fclose(fp);
	printf("size of myfile.txt: %ld bytes.\n", size);
	return 0; 

}
