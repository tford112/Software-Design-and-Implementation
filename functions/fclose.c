
#include <stdio.h>
#include <stdlib.h>

int main() {
	FILE *fp;
	fp = fopen("myfile.txt", "a");
	if (fp != NULL){
		fprintf(fp, "%s", "-fclose example-"); 
		fclose(fp);
	}
	else {
		perror("File cannot be found\n");
		exit(1);
	}
	return 0;
}
