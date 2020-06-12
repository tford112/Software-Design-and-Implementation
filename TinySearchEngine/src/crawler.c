/*

  FILE: crawler.c

  Description:

  Inputs: ./crawler [SEED URL] [TARGET DIRECTORY WHERE TO PUT THE DATA] [MAX CRAWLING DEPTH]

  Outputs: For each webpage crawled the crawler program will create a file in the 
  [TARGET DIRECTORY]. The name of the file will start a 1 for the  [SEED URL] 
  and be incremented for each subsequent HTML webpage crawled. 

  Each file (e.g., 10) will include the URL associated with the saved webpage and the
  depth of search in the file. The URL will be on the first line of the file 
  and the depth on the second line. The HTML will for the webpage 
  will start on the third line.

*/

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include "crawler.h"
#include "html.h"
#include "hash.h"
#include "header.h"
#include "gumbo.c"  // parser function located here 
#include "gumbo.h"

// Define the dict structure that holds the hash table 
// and the double linked list of DNODES. Each DNODE holds
// a pointer to a URLNODE. This list is used to store
// unique URLs. The search time for this list is O(n).
// To speed that up to O(1) we use the hash table. The
// hash table holds pointers into the list where 
// DNODES with the same key are maintained, assuming
// the hash(key) is not NULL (which implies the URL has
// not been seen before). The hash table provide quick
// access to the point in the list that is relevant
// to the curr URL search. 

DICTIONARY* dict = NULL; 
const char *URL_PREFIX = "https://home.dartmouth.edu"; 
int max_depth = 3; 



// (1) -- Command line processing on arguments 

void commandLine(int argc, char *argv[]) {
	if (argc != 4) {
		perror("Not enough arguments supplied\n");
		exit(1);
	}
	if (atoi(argv[3]) > max_depth) {
		perror("Depth is greater than max depth\n");
		exit(2); 
	}
	FILE *dir = fopen(argv[2], "r"); 
	if (dir == NULL) {
		perror("Directory to store HTMLs not supplied\n");
		exit(3);
	}
	fclose(dir); 


}

//(2) - initLists (properly intialize the Dictionary Hash Table) 

void initLists(){
	dict = (DICTIONARY*) malloc(sizeof(DICTIONARY));
	MALLOC_CHECK(dict);
	memset(dict, 0, sizeof(DICTIONARY)); 
	dict->start = NULL;
	dict->end = NULL; 
	for (int i =0; i < MAX_HASH_SLOT ; ++i) {
		dict->hash[i] = NULL; 
	}

}

// (3) -- 


/* 1. need to use wget to execute and download seedURL to temp file 
 * 2. to do this need to determine length of this temp file 
 *  (remember, it has to store all the HTML information so this will be a pretty long file )
 * 3. return pointer to buffer 
*/

char *getPage(char *seedURL, int curr_depth, char *target_directory) {
	char command[MAX_URL_LENGTH];  
	snprintf(command, MAX_URL_LENGTH, "wget %s -O buf.html", seedURL); 
	system(command);
	FILE *html = fopen("buf.html", "r"); 
	if (html == NULL) {
		perror("Wget didn't properly download to a temp file\n"); 
		exit(1);
	}
	fseek(html, 0, SEEK_END); // seek to the end of the file 
	int html_size = ftell(html);  // get current size of file after going to end 
	rewind(html);  	          // rewind all the way back to read in later to buffer 
	char *buf = malloc(sizeof(char) * html_size + 1); // +1 for null-terminator 
	MALLOC_CHECK(buf);
	memset(buf, 0, html_size); // need to initialize malloc'ed buffer before using 
	size_t result = fread(buf, sizeof(char), html_size, html); // read into buffer
	if (result != html_size) {
		fputs("reading error", stderr); 
		exit(2); 
	}
	fclose(html);   // cleaning up by closing the file stream and removing the temp file
	char remove[15]; 
	snprintf(remove, 15, "rm buf.html"); 
	system(remove);
	return buf;
}

// (4) -- extractURLS(page, SEED_URL) 

/* 1.) use the parser, GetNextURL (already provided by professors) to extract URLS from buffer (containing HTML page) 
 * 2.) add each URL to the url_list[] if the URL has the same URL_PREFIX ("http://www.cs.dartmouth.edu")  
 * 	-> This way we don't crawl other links in URLs and end up getting blocked (e.g. going to NY Times and crawling there 
 * 	because a URL we extracted told the crawler to go there) 
 * 3.) recall that the url_list is an array of pointers to char so need to malloc a buffer to store the correct extracted URLS 
 *
 * note: This course is outdated so the course recommended URL_PREFIX to be "http://www.cs.dartmouth.edu" but running the parser 
 *     on this seedURL only shows 1 viable link. I'm editing the prefix to be all of dartmouth 
 */

char **extractURLS(char *page, char *seedURL) {
	char *url_results = all_urls(&page);  // get all the URLS from the Gumbo parser 
	int num_of_urls = 0;   // get the number of URL strings we need to malloc later 
	for (int i = 0; i < strlen(url_results); ++i) {
		if (url_results[i] == '\n') {  //allocating more char* memories than will actually be needed 
			++num_of_urls;
		}
	}

	// This is the table that keeps pointers to a list of URL extracted
	// from the current HTML page.
	// reminder -> can't return array from stack so need a pointer to pointer 
	char **url_list = malloc(sizeof(char*) * num_of_urls);
	MALLOC_CHECK(url_list); 
	for (int i = 0; i < num_of_urls; ++i) {
		url_list[i] = malloc(sizeof(char) * MAX_URL_LENGTH); 
		MALLOC_CHECK(url_list[i]); 
	}

	int len = strlen(url_results); 
	int n = 0;
	int single_index = 0; 
	char single[MAX_URL_LENGTH] ;  // buffer for a single url to be placed here 
	memset(single, 0, MAX_URL_LENGTH); 

	int url_list_index = 0; 
	snprintf(url_list[url_list_index++], MAX_URL_LENGTH, "%s", seedURL);// first entry will be the seedURL  
	int rel_num_urls = 0; 
	while (n < len-1) {
		if (url_results[n] == '\n') {  	// detect line break, store/capture our single URL and check if it is valid (e.g. it's part of the seedURL) 
			single[single_index] = '\0'; 
			char *found = strstr(single, URL_PREFIX);  // detect substring of similar URL to seed
			if (found) {
				/*
				char *put_result= calloc(strlen(single), sizeof(char)); 
				MALLOC_CHECK(put_result); 
				
				snprintf(put_result, sizeof(single), "%s", single);  // putting the char array values into a char* to be put into url_list 
				*/
				snprintf(url_list[url_list_index++], MAX_URL_LENGTH, "%s", single); 
				++rel_num_urls;
				/*url_list[url_list_index++] = put_result; // updating a pointer to char to point to this dynamically allocated char*/
			}
			memset(single, 0, MAX_URL_LENGTH);  	// reset the buffer 
			++n; 					// go to the next line 
			single_index = 0;  			// reset the buffer point 
		}
		else { 
			single[single_index++] = url_results[n++]; 
		}
	}	
	// Not enough to reallocate-- need to also set these pointers to NULL 
	// otherwise the next function will keep iterating and trying to dereference empty pointers 
	// because they're all set to 0 after the realloc instead of explicit null. 
	url_list = realloc(url_list, sizeof(char*) * rel_num_urls);
	for (int i = rel_num_urls; i <= num_of_urls ; ++i) {  
		url_list[i] = NULL; 
	}
	MALLOC_CHECK(url_list);
	return url_list; 
}

//(5) *updateListLinkToBeVisited(URLsLists, curr_depth + 1)*  For all the URL 
//    in the URLsList that do not exist already in the dictionary then add a DNODE/URLNODE 
//    pair to the DNODE list. 
//  1.) Figure out uniqueness of url (use the hash function -> if hash into Dictionary and is null, then is unique) 
//

void updateListLinkToBeVisited(char **url_list, int depth) {
	// initialize first DNODE with URLNODE 

	int n =0; 

	/* allocate memory for 2 Dnodes, which will represent the start and finish of the doubly linked list. 
	 * We can assign the hash table's start and finish to these two memory locations of the dnodes. 
	 * We can also assign their next and prev pointers to point to these allocated dnodes as well 
	 */
	dict->start = malloc(sizeof(DNODE)); 
	MALLOC_CHECK(dict->start); 
	dict->end = malloc(sizeof(DNODE)); 
	MALLOC_CHECK(dict->end); 

	dict->start->next = dict->end; 
	dict->start->prev = NULL; 
	dict->end->prev = dict->start; 
	dict->end->next = NULL; 
	DNODE *reg_curr = malloc(sizeof(DNODE)); 
	MALLOC_CHECK(reg_curr); 
	reg_curr = dict->start; 
	unsigned long hash_value = 0; 

	while (url_list[n]) { 
		hash_value = hash1(url_list[n]) % MAX_HASH_SLOT ;
		printf("%s \t: %lu\n", url_list[n], hash_value); 
		
		// check if DICTIONARY has null space for hash value 
		if (dict->hash[hash_value] == NULL ) { 
			
			URLNODE *node = malloc(sizeof(URLNODE)); 
			MALLOC_CHECK(node); 
			node->depth = depth; 
			node->visited = 0; 
			snprintf(node->url, MAX_URL_LENGTH, "%s", url_list[n]); 

	// need malloc(DNODE) part because I kept running into errors trying to 
	// directly initialize "dict->hash[hash_value]->data = node". Need to allocate memory 
	// for a DNODE first most likely before setting values to it...even though I thought 
	// it was already allocated from the DICTIONARY 
	//		dict->start = dict->end = malloc(sizeof(DNODE)); 
			dict->hash[hash_value] = malloc(sizeof(DNODE)); // initialize to malloc(dnode) 
			MALLOC_CHECK(dict->hash[hash_value]); 
			dict->hash[hash_value]->data = node; 
			snprintf(dict->hash[hash_value]->key, KEY_LENGTH, "%s", node->url); 
	/* where our doubly linked list updates itself with the insertion */ 
			dict->hash[hash_value]->next = dict->end; 
			dict->hash[hash_value]->prev = reg_curr; 
			reg_curr->next = dict->hash[hash_value]; 
			dict->end->prev = dict->hash[hash_value]; 
			if (n == 0) {
				printf("setting the start\n"); 
				dict->start = reg_curr->next; 
				printf("confirming start: %s\n", dict->start->key);
			} 
			reg_curr = reg_curr->next;  // update the curr pointer to next node  

		}
		// collision occurred with value already in Dictionary for that hash index			    
		
		else if (dict->hash[hash_value]) {  
			unsigned long current_hash = hash_value; 
			DNODE *coll_curr = dict->hash[hash_value]; 
			// Collision could occur because
			// 1.) same exact url link (so not unique) 
			// 2.) genuine collision occurred of different values mapping to the same hash index 
			bool proceed = true; 
			while (coll_curr) {
				if (strcmp(coll_curr->key, url_list[n]) == 0) { // same value (not unique) 
					proceed = false; 
					break; 
				} 
				else {
					hash_value = hash1(coll_curr->next->key); 
					if (hash_value != current_hash) {
						break; 
					}
				}
				coll_curr = coll_curr->next;
			}
			if (proceed) { 
				URLNODE *url_node = malloc(sizeof(URLNODE)); 
				MALLOC_CHECK(url_node); 
				url_node->depth = depth; 
				url_node->visited = 0; 
				snprintf(url_node->url, MAX_URL_LENGTH, "%s", url_list[n]); 

				DNODE *new = malloc(sizeof(DNODE)); 
				MALLOC_CHECK(new); 
				new->data = url_node;  // set DNODE pointer to data to point to new URLNODE 
				new->next = coll_curr->next; 
				new->prev = coll_curr; 
				snprintf(new->key, KEY_LENGTH, "%s", url_list[n]); 
				coll_curr->next = new; 
			}
		}
		
		++n;
	}
	dict->end = dict->hash[hash_value];  	
	free(reg_curr); 

}

// some of the functionality from below might be removed later 
// but need to make sure to only update the visited value for the node that has the seedURL 
// in the dictionary linked list 
// I don't want to mess around with the pointers that dict uses so I use another DNODE *pointer 
// (like reg_curr in the above function) to traverse and modify the values 
void setURLasVisited(char *seedURL) { // pass in the SEED URL 
	unsigned long hash_value = hash1(seedURL) % MAX_HASH_SLOT; 
	unsigned long same_hash = hash_value; 
	DNODE *pointer = malloc(sizeof(DNODE)); 
	MALLOC_CHECK(pointer); 
	pointer = dict->hash[hash_value]; 
	while (pointer) { 
		if (strcmp(pointer->data->url, seedURL) == 0) {
			pointer->data->visited = 1; 
		}
		pointer = pointer->next; 
		hash_value = hash1(pointer->key); 
		if (hash_value != same_hash) {
			break;
		}
	}

	free(pointer); 
}

/* This function checks the doubly linked list for the next node to visit that hasn't already been visited 
 * and get that address for the crawler to use. 
 */
char *getAddressFromTheLinksToBeVisited(int depth) {
	DNODE *current = malloc(sizeof(DNODE)); 
	MALLOC_CHECK(current); 
	current = dict->start; 
	while (current) {
		if (current->data->visited) {
			current = current->next; 
		}
		else {
			return current->data->url; 
		}
	}
	free(current); 
	return NULL;
}


int main(int argc, char *argv[]) {
	commandLine(argc, argv); 
	initLists() ;
	char *seedURL = argv[1];  // example would be "www.cs.dartmouth.edu"
	int curr_depth = atoi(argv[2]); 
	char *target_dir = argv[3];
	char *page = getPage(seedURL, curr_depth, target_dir) ;
	char **url_list = extractURLS(page, seedURL);
	updateListLinkToBeVisited(url_list, 0); 
	setURLasVisited(seedURL); 
	/*
	snprintf(url_to_visit, MAX_URL_LENGTH, "%s", getAddressFromTheLinksToBeVisited(0)); 
	printf("Next url to visit is: %s\n", url_to_visit);

	char url_to_visit[MAX_URL_LENGTH]; 
	memset(url_to_visit, 0, MAX_URL_LENGTH); 
	char *url_to_visit = malloc(sizeof(char) * MAX_URL_LENGTH); 
	MALLOC_CHECK(url_to_visit); 
	*/
	while (getAddressFromTheLinksToBeVisited(curr_depth)) { 
		if (curr_depth > max_depth) {
			// for urls over max_depth, set them to be visited and continue 
			setURLasVisited(url_to_visit);  // mark current url visited 
			continue; 
		}
		// get html into string and return as page. Also save as a file into target dir 
		char *page = getPage(seedURL, curr_depth, target_dir);	
		if (page == NULL) {
			printf("warning! Cannot crawl current url. Most likely bad URL link. Continuing on..\n"); 
			setURLasVisited(url_to_visit); // mark bad url as visited 
			continue; 
		}
		char **url_list = extractURLS(page, seedURL); 
		free(page); 
		updateListLinkToBeVisited(url_list, curr_depth); 
		char *url_to_visit = getAddressFromTheLinksToBeVisited(curr_depth++); 
		setURLasVisited(url_to_visit); 
		sleep(INTERVAL_PER_FETCH); 
	}
	

/*	

	//printf("page is:\n %s", page); 
//	free(page); 
*/


	return 0;
}

