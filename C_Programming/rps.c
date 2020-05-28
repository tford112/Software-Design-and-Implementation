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

typedef enum {ROCK, PAPER, SCISSORS, EXIT} choice; 

char *my_getline();
int compare_inp(char *);
int computer_move() ;
void game_result(int player, int cpu, int *pwin, int *cwin, int *tie) ;
char *translate(int comp_move);
	
int main(int argc, char *argv[]){
	int player_win = 0; 
	int player_move = -1;
	int comp_win = 0; 
	int tie = 0 ;
	int start = 1; 
	char *comp_res;
	printf("Please enter rock, paper, scissors\n"); 
	while (start) {
		do {
			char *inp = my_getline();
			player_move = compare_inp(inp); 
			free(inp);
		} while (player_move == ERROR); 

		if (player_move == EXIT) {
			printf("Game status: \n Win:  %d\n Lose:  %d\n Tie:  %d\n Total:  %d\n", \
					player_win, comp_win, tie, player_win + comp_win + tie); 
			exit(0);
		}
		int comp_move = computer_move();
		char *comp_res = translate(comp_move);
		if (comp_res == NULL){
			perror("Not enough memory for comp move\n");
			return ERROR;
		}
		printf("Computer played: %s\n", comp_res);
		free(comp_res); 
		
		game_result(player_move, comp_move, &player_win, &comp_win, &tie); 
	}

	return 0;
}

char *translate(int comp_move) {
	char *translate = malloc(sizeof(char)*SIZE/10); // buffer to hold string value (only need 10 slots or so)
	switch (comp_move) {
		case ROCK:
			snprintf(translate, SIZE/10, "%s", "ROCK");
			break;
		case PAPER:
			snprintf(translate, SIZE/10, "%s", "PAPER"); 
			break;
		case SCISSORS:
			snprintf(translate, SIZE/10, "%s", "SCISSORS"); 
			break;
	}
	return translate;
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

// compare player input to valid options 
int compare_inp(char *inp) {
	if (!strcmp(inp, "ROCK")) {
		return ROCK;
	}
	else if (!strcmp(inp, "PAPER")) {
		return PAPER;
	}
	else if (!strcmp(inp, "SCISSORS")){
		return SCISSORS;
	} 
	else if (!strcmp(inp, "E")) {
		return EXIT;
	}
	printf("Please enter valid rock, paper, scissors value\n"); 
	return ERROR;
}

// computer randomly chooses move 
int computer_move() {
	srand(time(0)); // initializes RNG with current time as seed 
	int intermediary_random = rand() % 1000; 
	return intermediary_random % 3; 
}

// game result between player choice and computer randomly-generated move
void game_result(int player, int cpu, int *pwin, int *cwin, int *tie) {
	switch (player) {
		case ROCK:  // you played rock 
			if (cpu == PAPER) {
				printf("CPU wins!\n"); 
				(*cwin)++; 
				break;
			}
			else if (cpu == SCISSORS) {
				printf("You win!\n"); 
				(*pwin)++;
				break;
			}
			else {
				printf("Tied!\n"); 
				(*tie)++;
				break;
			}
		case PAPER: // you played paper 
			if (cpu == SCISSORS) {
				printf("CPU wins\n");
				(*cwin)++;
				break;
			}
			else if (cpu == ROCK) {
				printf("You win!\n"); 
				(*pwin)++; 
				break;
			}
			else {
				printf("Tied!\n"); 
				(*tie)++;
				break;
			}
		case SCISSORS: // you played scissors  
			if (cpu == ROCK) {
				printf("CPU wins\n");
				(*cwin)++;
				break;
			}
			else if (cpu == PAPER) {
				printf("You win!\n"); 
				(*pwin)++; 
				break;
			}
			else {
				printf("Tied!\n"); 
				(*tie)++;
				break;
			}
	}
}


		
