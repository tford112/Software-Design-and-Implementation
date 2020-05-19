/* Implement the classic rock paper scissors game. This question challenges you in what is sometimes considered the black art of software decomposition. Given the requirements of this simple, well-known game (see how the program performs below), how do we decompose the game into main() and its associated functions. How does the code break down into functions? 
 * The computer should randomly make its choice when playing the user.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include <unistd.h>
#define SIZE 100
#define ERROR -1

char *my_getline();
int compare_inp(char *);
int comp_move(); 

	

int main(int argc, char *argv[]){
	int player_win = 0; 
	int comp_win = 0; 
	int start = 1; 
	while (start) {
		char *inp = my_getline();
		int player_move = compare_inp(inp); 
		int comp_move = comp_move();
		game_result(player_move, comp_move, &player_win, &comp_win); 
		printf("Please enter rock, paper, scissors\n"); 

	}
	return 0;
}

char* my_getline() {
	int nch = 0; 
	int c; 
	char *out = malloc(sizeof(char) * SIZE+1);  // +1 for null-terminator
	if (out == NULL) {
		perror("Not enough space\n");
		exit(1);
	}
	while ((c = getchar()) != EOF) {
		if (c == '\n') {
			break;
		}
		if (nch != SIZE) {
			out[nch++] = toupper(c); 
		}
	}
	if (c == EOF || c == 0) {
		exit(1);
	}
	out[nch] = '\0'; 
	return out; 
}

int compare_inp(char *inp) {
	if (!strcmp(inp, "ROCK")) {
		return 0;
	}
	else if (!strcmp(inp, "PAPER")) {
		return 1; 
	}
	else if (!strcmp(inp, "SCISSORS")){
		return 2; 
	} 
	else if (!strcmp(inp, "E")) {
		return 3; 
	else {
		perror("Please provide a valid choice: rock, paper, or scissors\n"); 
		return ERROR;
	}
}

int comp_move() {
	srand(time(0)); // initializes RNG with current time as seed 
	int intermediary_random = rand() % 1000; 
	return intermediary_random % 3; 
}

void game_result(int player, int cpu, int *pwin, int *cwin) {
	switch (player) {
		case 0:
			if (cpu == 1) *cwin++; 
			else if (cpu == 2) *pwin++; 
			break;
		case 1: 
			if (cpu == 2) *cwin++;
			else if (cpu == 0) *pwin++; 
			break; 
		case 2: 
			if (cpu == 0) *cwin++;
			else if (cpu == 1) *pwin++;
			break; 
	}
}


		
