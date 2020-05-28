
#include <stdio.h>
#include <stdlib.h>
#include <string.h> 
#include <unistd.h>

#define SIZE 100

void echo();

/*getopt(argc, argv, [options]) checks if there is an option. If there isn't will return a -1. 
 * Otherwise will encounter option and try to parse it. If it encounters unknown option ("?") will 
 * catch error and exit */ 
int main(int argc, char *argv[]) {
	int option_index ; 
	while ((option_index = getopt(argc, argv, "n")) != -1) {
		switch (option_index) {
			case 'n':
				echo(); 
				exit(0);
			case '?':
				printf("unknown option\n");
				exit(1);
		}
	}
	echo();
}

void echo(){
	int c; 
	char out[SIZE];
	memset(out, 0, SIZE);
	int nch = 0; 
	while ((c = getchar()) != EOF) {
		if (c == '\n') {
			break;
		}
		if (nch != SIZE) {
			out[nch++] = c; 
		}
	}
	if (c == EOF || c == 0) {
		exit(1); 
	}
	out[nch] = '\0'; // null-terminate 
	printf("%s\n", out);
}

