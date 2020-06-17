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
#include <unistd.h>
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
//const char *URL_PREFIX = "https://web.cs.dartmouth.edu/";
int HASHES[MAX_HASH_SLOT]; 
FILE *out; 
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

int *initLists(){
	dict = (DICTIONARY*) malloc(sizeof(DICTIONARY));
	MALLOC_CHECK(dict);
	
	memset(HASHES, 0, MAX_HASH_SLOT); 
	int *gh_idx = malloc(sizeof(int));  
	MALLOC_CHECK(gh_idx); 
	*gh_idx = 0; 

	for (int i =0; i < MAX_HASH_SLOT ; ++i) {
		dict->hash[i] = NULL; 
	}
	return gh_idx; 
}

// (3) --  getPage  -> using wget 

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


	// BUG HERE -> need to properly allocate buf otherwise is a stack variable not a pointer 
		
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

char **extractURLS(char *page, char *seedURL, int *url_list_length) {
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
	while (n < len-1) {
		if (url_results[n] == '\n') { 	// detect line break, store/capture our single URL and check if it is valid 
			single[single_index] = '\0'; 
			char *found = strstr(single, URL_PREFIX);  // detect substring of similar URL to seed
			if (found) {
				char *detect_pdf = strstr(single, ".pdf");   // don't extract pdfs 
				if (detect_pdf) {
					;
				}
				else {
				       	snprintf(url_list[url_list_index], MAX_URL_LENGTH, "%s", single); 
					++url_list_index; 
				}
			}
			memset(single, 0, MAX_URL_LENGTH);  	// reset the buffer 
			++n; 					// go to the next line 
			single_index = 0;  			// reset the buffer point 
		}
		else { 
			single[single_index++] = url_results[n++]; 
			if (single_index % 500 == 0) {  // gumbo parser can still return bad URL parsing. Need to break out
				char *detect_bad = strstr(single, "https://"); 

				fprintf(out, "DETECTED BAD URL");
				fprintf(out, "single at this point: %s\n\n", single);
				if (detect_bad == NULL) {
					break;
				}
			}
		}
	}	
	// Not enough to reallocate-- need to also set these pointers to NULL 
	// otherwise the next function will keep iterating and trying to dereference empty pointers 
	// because they're all set to 0 after the realloc instead of explicit null. 
	
	for (int i = url_list_index ; i <= num_of_urls ; ++i) {  
		url_list[i] = NULL; 
	}
	url_list = realloc(url_list, sizeof(char*) * url_list_index);
	MALLOC_CHECK(url_list);

	*url_list_length = url_list_index;

	return url_list; 
}

//(5) updateListLinkToBeVisited(URLsLists, curr_depth + 1)  

void updateListLinkToBeVisited(char **url_list, int url_list_length, int depth, int*gh_idx) {
	//fprintf(out, "current GHIDX: %d\n", *gh_idx);
	int n =0; 
	unsigned long hash_value = 0; 

	int curr_hash; 
	while ((url_list[n]) && (url_list_length >0)) {
		if ((url_list[n] == NULL) || (url_list[n] == '\0')) {
			break;
		}

		hash_value = hash1(url_list[n]) % MAX_HASH_SLOT ;
	//	printf("%s \t: %lu\n", url_list[n], hash_value); 
		curr_hash = hash_value;  //
		
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
			dict->hash[hash_value] = malloc(sizeof(DNODE)); // initialize to malloc(dnode) 
			MALLOC_CHECK(dict->hash[hash_value]); 
			dict->hash[hash_value]->data = node; 
			snprintf(dict->hash[hash_value]->key, KEY_LENGTH, "%s", node->url); 
			dict->hash[hash_value]->next = NULL; 
			dict->hash[hash_value]->prev = NULL; 
			HASHES[(*gh_idx)++] = curr_hash;   // store hash_value in hash index array 
			fprintf(out, "currdepth: %d hash_val->%s\n", dict->hash[hash_value]->data->depth, dict->hash[hash_value]->data->url);



		}
		// collision occurred with value already in Dictionary for that hash index			    
		
		else if (dict->hash[hash_value]) {  
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
				snprintf(new->key, KEY_LENGTH, "%s", url_list[n]); 
				new->next = NULL;
				new->prev = coll_curr->prev; 
				coll_curr->prev->next = new; 
				HASHES[(*gh_idx)++] = curr_hash;   // store hash_value in hash index array 
				fprintf(out, "currdepth: %d hash_val->%s\n", dict->hash[hash_value]->data->depth, dict->hash[hash_value]->data->url);



			}
		}
		
		++n;
		--url_list_length; 
	}

}

// setURLasVisited -> set url as visited so will not visit again 

void setURLasVisited(char *seedURL, int *curr_hash_idx) { // pass in the SEED URL 
	unsigned long hash_value = hash1(seedURL) % MAX_HASH_SLOT;   // compute hash value for url parameter 
	unsigned long same_hash = hash_value; 
	DNODE *pointer = dict->hash[hash_value];     // access hash table for that URL hash value 
	while (pointer) { 
		if (strcmp(pointer->data->url, seedURL) == 0) {    // found the URL that share the same hash key (collision) 
			pointer->data->visited = 1; 
			(*curr_hash_idx)++; 
			break;
		}
		pointer = pointer->next; 
		hash_value = hash1(pointer->key);
		if (hash_value != same_hash) {
			break;
		}
	}

	//free(pointer);   -> CAN'T FREE YET! otherwise would lead to a dangling reference 
}

/* This function checks the doubly linked list for the next node to visit that hasn't already been visited 
 * and get that address for the crawler to use. 
 */
char *getAddressToBeVisited(int *depth, int *curr_hash_idx) {
	int hash_value = HASHES[*curr_hash_idx]; 
	DNODE *current = dict->hash[hash_value]; 
	while (current) {
		if (current->data->visited) { // positive value means visited so skip 
			current = current->next; 
		}
		else {
			return current->data->url; 
		}
	}

	return NULL;
}


int main(int argc, char *argv[]) {
	out = fopen("logger.txt", "w+"); 
	commandLine(argc, argv); 
	int *gh_idx = initLists() ;
	char *seedURL = argv[1];  // example would be "www.cs.dartmouth.edu"
	char *target_dir = argv[2];
	int max_depth  = atoi(argv[3]); 
	char url_to_visit[MAX_URL_LENGTH]; 
	memset(url_to_visit, 0, MAX_URL_LENGTH); 
	
	int curr_depth = 0; 
	char *page = getPage(seedURL, curr_depth, target_dir) ;
	int url_list_length = 0;  // added variable 
	int curr_hash_to_view = 0; 
	char **url_list = extractURLS(page, seedURL, &url_list_length);
	updateListLinkToBeVisited(url_list, url_list_length, curr_depth, gh_idx); 
	setURLasVisited(seedURL, &curr_hash_to_view); 

	while (snprintf(url_to_visit, MAX_URL_LENGTH, "%s", getAddressToBeVisited(&curr_depth, &curr_hash_to_view)) != 0){ 
		if (strcmp(url_to_visit, "(null)") == 0) {
			break;
		}
		printf("Next url to visit is: %s\n", url_to_visit);
		fprintf(out, "Next url to visit is: %s\n", url_to_visit);
		if (curr_depth > max_depth) {
			// for urls over max_depth, set them to be visited and continue 
			setURLasVisited(url_to_visit, &curr_hash_to_view);  // mark current url visited 
			continue; 
		}
		// get html into string and return as page. Also save as a file into target dir 
		char *page = getPage(url_to_visit, curr_depth, target_dir);	
		if ((page == NULL) || (page[0] == '\0')) {
			printf("warning! Cannot crawl current url. Most likely bad URL link. Continuing on..\n"); 
			fprintf(out, "warning! Cannot crawl current url. Most likely bad URL link. Continuing on..\n"); 
			setURLasVisited(url_to_visit, &curr_hash_to_view); // mark bad url as visited 
			continue; 
		}
		char **url_list = extractURLS(page, url_to_visit, &url_list_length); 
//		free(page);
		updateListLinkToBeVisited(url_list, url_list_length, curr_depth+1, gh_idx);  // here the current depth increments 
		setURLasVisited(url_to_visit, &curr_hash_to_view); 
		memset(url_to_visit, 0, MAX_URL_LENGTH); 
		sleep(INTERVAL_PER_FETCH+4); 
		fprintf(out, "CURRENT HASHES: \n"); 
/*
		int s = 0 ; 
		while (HASHES[s] != 0) {
			int hv = HASHES[s];
			fprintf(out, "currdepth: %d hash_val->%s\n", dict->hash[hv]->data->depth, dict->hash[hv]->data->url);
			++s;
		}
		fprintf(out, "hash index is %d\n", *gh_idx); 
*/

	}
	printf("Finished!\n");
	fprintf(out, "Finished\n");
	fclose(out);	
	
	return 0;
}

