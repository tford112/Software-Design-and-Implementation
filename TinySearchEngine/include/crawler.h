#ifndef _CRAWLER_H_
#define _CRAWLER_H_


// *****************Impementation Spec********************************
// File: crawler.c
// This file contains useful information for implementing the crawler:
// - DEFINES
// - MACROS
// - DATA STRUCTURES
// - PROTOTYPES

// DEFINES

// The max length of each URL path.

#define INTERVAL_PER_FETCH 1

// The URL we crawled should only starts with this prefix. 
// You could remove this limtation but not before testing.
// The danger is a site may block you

/* REPLACED */ 
//#define URL_PREFIX "https://home.dartmouth.edu" 
extern const char *URL_PREFIX ;
extern int max_depth; 

// Set some magic numbers. These are large values.

// Unlikely to have more than an a URL longer that 1000 chars.
// The KEY is the same as a URL. The term KEY is just a
// general Dictionary/hash function term 

#define MAX_URL_LENGTH 1000
#define KEY_LENGTH 1000

// Make the hash large in comparison to the number of URLs found
// for example depth of 2 on www.cs.dartmouth.edu approx 200.
// This will minimize collisions and mostly likely slots will be
// empty of have one or two DNODES. Access is O(1). Fast.

#define MAX_HASH_SLOT 10000

// Unlikely to have more than an 1000 URLs in page
 
#define MAX_URL_PER_PAGE 10000

// DATA STRUCTURES. All these structures should be malloc 'd

// This is the key data structure that holds the information of each URL.

typedef struct _URL{
  char url[MAX_URL_LENGTH];      // e.g., www.cs.dartmouth.edu
  int depth;                     //  depth associated with this URL.
  int visited;                   //  crawled or not, marked true(1), otherwise false(0)
} __URL;

typedef struct _URL URLNODE;


// Dictionary Node. This is a general double link list structure that
// holds the key (URL - we explained into today's lecture why this is there)
// and a pointer to void that points to a URLNODE in practice. 
// key is the same as URL recall.

typedef struct _DNODE {
  struct _DNODE *next;
  struct _DNODE *prev;
  struct _URL *data;     //  actual data points to URLNOD
  char key[KEY_LENGTH]; //  actual (URL) key 
} __DNODE;

typedef struct _DNODE DNODE;

// The DICTIONARY holds the hash table and the start and end pointers into a double 
// link list. This is a unordered list with the exception that DNODES with the same key (URL) 
// are clusters along the list. So you hash into the list. Check for uniqueness of the URL.
// If not found add to the end of the cluster associated wit the same URL. You will have
// to write an addElement function.

typedef struct _DICTIONARY {
  DNODE *hash[MAX_HASH_SLOT]; // the hash table of slots, each slot points to a DNODE
  DNODE *start;               // start of double link list of DNODES terminated by NULL pointer
  DNODE *end;                 // points to the last DNODE on this list
} __DICTIONARY;

typedef struct _DICTIONARY DICTIONARY;
  
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
// to the current URL search. 

extern DICTIONARY *dict;

/*
// This is the table that keeps pointers to a list of URL extracted
// from the current HTML page. NULL pointer represents the end of the
// list of URLs.

char *url_list[MAX_URL_PER_PAGE];
extern int url_list_index; 
*/

//PROTOTYPES used by crawler.c You have to code them.

// getPage: Assumption: if you are dowloading the file it must be unique. 
// Two cases. First its the SEED URL so its unique. Second, wget only getpage 
//once a URL is computed to be unique. Get the HTML file saved in TEMP 
// and read it into a string that is returned by getPage. Store TEMP
// to a file 1..N and save the URL and depth on the first and second 
// line respectively.

char *getPage(char* url, int depth,  char* path);

// extractURL: Given a string of the HTML page, parse it (you have the code 
// for this GetNextURL) and store all the URLs in the url_list (defined above).
// NULL pointer represents end of list (no more URLs). Return the url_list

char **extractURLs(char* html_buffer, char* current, int*);

// setURLasVisited: Mark the URL as visited in the URLNODE structure.

void setURLasVisited(char* url, int *hash_value);

// updateListLinkToBeVisited: Heavy lifting function. Could be made smaller. It takes
// the url_list and for each URL in the list it first determines if it is unique.
// If it is then it creates a DDNODE (using malloc) and URLNODE (using malloc).
// It copies the URL to the URNODE and initialises it and then links it into the
// DNODE (and initialise it). Then it links the DNODE into the linked list dict.
// at the point in the list where its key cluster is (assuming that there are
// elements hashed at the same slot and the URL was found to be unique. It does
// this for *all* URL in the ur-list

void updateListLinkToBeVisited(char **url_list, int, int depth, int*);

// getAddressFromTheLinksToBeVisited: Scan down thie hash table (part of dict) and
// find the first URL that has not already been visit and return the pointer 
// to that URL. Note, that the pointer to the depth is past too. Update the
// depth using the depth of the URLNODE that is next to be visited. 

char *getAddressToBeVisited(int *depth, int *hash_value);

#endif

