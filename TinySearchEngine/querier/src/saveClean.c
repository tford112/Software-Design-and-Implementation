
#include <stdio.h> 
#include <stdlib.h> 
#include "../include/query.h" 
#include "../include/allocate.h"

void saveIndex(INVERTED_INDEX* index, char* filename, FILE* logger) {
	int cur, doc_count;
	cur = 0; 
	doc_count = 0; 
	FILE *index_output = openFile(filename, "wb"); 
	while (cur < MAX_HASH_SLOT) {
		if (index->hash[cur]) {   			// we won't know how many values in the hash are NULL 
			DocNode* doc_page = index->hash[cur]->page;
			DocNode* cur_page = index->hash[cur]->page; 
			while (doc_page) {  			 // we need to first print out the overall Doc Count before going into each specific doc
				++doc_count; 	
				doc_page = doc_page->next; 
			}
			fprintf(index_output, "\n%s %d ", index->hash[cur]->word, doc_count); 
			while (cur_page) {  			 // get document id and frequency for index.dat 
				fprintf(index_output, "%d %d ", cur_page->docId, cur_page->page_word_frequency); 
				cur_page = cur_page->next; 
			}
			fprintf(logger, "Total doc count for word \"%s\": %d\n", index->hash[cur]->word, doc_count);
			doc_count = 0; 				 // reset for next word 
		}
		++cur; 
	}
	fclose(index_output); 
}

void cleanIndex(INVERTED_INDEX* index, FILE* logger) {
	fprintf(logger, "Now cleaning..\n"); 
	for (int i =0; i < MAX_HASH_SLOT; ++i) {
		WordNode* wnode = index->hash[i]; 
		if (wnode == NULL) {
			free(wnode); 
			wnode = NULL; 
		}
		else {
			DocNode* dnode = wnode->page; 
			while (dnode) {     			 // free every allocated document node 
				DocNode* freedNode = dnode;
				free(freedNode); 
				freedNode = NULL; 
				dnode = dnode->next;  		 // need to make sure they are all set to NULL after freeing

			}
			free(wnode); 
			wnode = NULL; 
		}
	}
	free(index); 
	fputs("Successfully cleaned\n", logger);
	index = NULL;
}




