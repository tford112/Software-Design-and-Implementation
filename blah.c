
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define SIZE 1 

int main(int argc, char *argv[]) {
	int **hashes = malloc(sizeof(int*) * SIZE); 
	for (int i = 0; i < SIZE; ++i) {
		hashes[i] = NULL;
	}
	int n = 12; 
	int *p = (int*)malloc(sizeof(int)); 
	if (p == NULL) {
		perror("no memory\n");
		exit(1);
	}
	*p = 12;    
	hashes[0] = p;
	printf("hash value at hash index 1: %d", *hashes[0]); 


	int *r = &n;   // cannot do this! because setting memory and setting it to a stack variable which will be cleaned out
	(*r)++;	
	printf("p is now %d\n", n);

	int *w = malloc(sizeof(int));
	if (w == NULL){
		perror("not enough memory\n");
		exit(2);
	}
	*w = 24; 
	printf("pointer value is %d\n", *w);

	char *str;

	/* Initial memory allocation */
	str = (char *) malloc(15);
	strcpy(str, "tutorialspoint");
	printf("String = %s,  Address = %u\n", str, str);
	free(str);

	int *array = (int*) malloc(sizeof(int) * 10);
        if (array == NULL) {
        	fprintf(stderr, "Unable to allocate enough memory for array!\n");
        return -1;
	}
	free(array);
	free(p);

}
