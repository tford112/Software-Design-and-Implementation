
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define SIZE 80 

int main() {
	char str1[] = "CS50 brought down the departmental web + mail servers!\n"; 
	char str2[SIZE];
	char str3[SIZE]; 

	/* copy to sized buffer (overflow safe): */ 
	strncpy(str2, str1, sizeof(str2)) ; 

	// partial copy (only 16 chars) 
	strncpy(str3, str2, 16); 
	str3[16] = '\0'; 

	puts(str2);
	puts(str3); 
	return EXIT_SUCCESS; 
}

