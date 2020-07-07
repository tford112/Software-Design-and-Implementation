// Copyright 2013 Google Inc. All Rights Reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// Author: jdtang@google.com (Jonathan Tang)
//
//
/*  gumbo-parser is a C library that can parse HTML5. The search_for_links()
 *  is mostly taken from their repo with only some few modifications to output 
 *  and the GumboNode* casting of the children->data[i]. This function is printing 
 *  out all the hrefs and recursively calls itself until there are no more href 
 *  elements in the html 
 * 
 *  The goal is to capture all the anchor tags with dartmouth hrefs and add them to 
 *  the valid URLS (for further crawling if depth >1 and generally for indexing) 
 *
 *  The library was needed as the original parser provided by the professor doesn't 
 *  work. 
 */

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include "gumbo.h"

#define LENGTH 1000

char url_buf[LENGTH]; 

char *allocateBufferForFile(FILE *fp) {
	if (fp == NULL) { 			// error checking file 
		perror("File not found\n");
		exit(2);
	}

	fseek(fp, 0, SEEK_END);   // get the number of bytes from file input to malloc
	int total_bytes = ftell(fp); 
	rewind(fp); 

	char *buf = malloc(sizeof(char) * total_bytes+1);  // allocate just enough bytes to handle the entire html input 

	if (buf == NULL) {			// error checking buffer{
		perror("Not enough space\n");
		exit(3);
	}
	memset(buf, 0, total_bytes); 
	int return_fread = fread(buf, sizeof(char), total_bytes, fp);  
	if (return_fread != total_bytes) {
		perror("Not all bytes read\n"); 
		exit(4); 
	}
	fclose(fp); 
	return buf; 
}


static void search_for_links(GumboNode* node, FILE *store) {
  if (node->type != GUMBO_NODE_ELEMENT) {
    return;
  }
  GumboAttribute* href;
  if (store == NULL) {
	  perror("Error occurred in file open\n");
	  exit(3); 
  }
  if (node->v.element.tag == GUMBO_TAG_A &&
      (href = gumbo_get_attribute(&node->v.element.attributes, "href"))) {
	  if (strlen(href->value) < LENGTH) {
		  snprintf(url_buf, LENGTH, "%s\n", href->value); // put the url link in buffer
		  fputs(url_buf, store); // store the buffer value into file 
	  }
	  else {
		  perror("One of the URLs exceeded acceptable length\n"); 
	  }
  }

  GumboVector* children = &node->v.element.children;
  for (unsigned int i = 0; i < children->length; ++i) {
    search_for_links((GumboNode*) children->data[i], store);
  }
}



char *all_urls(char **html_buf) { 

	GumboOutput* output = gumbo_parse(*html_buf);  // as a reminder, need the buf anyway because gumbo_parse only takes char* inputs 

	/* Set up a buffer to store all the URLS stored in FILE *stream
	 * after search function goes through the gumbo_parsed buffer.
	*/

	/* Important to use binary stream because can support more of the wonky characters that 
	 * can come up with reading the HTML file. Text streams also can fail to handle lines 
	 * more than 254 chars long. 
	 *
	 * Binary streams ae simply a long series of characters while the text stream is divided 
	 * into lines based on '\n'. 
	 */
	FILE *store = fopen("stored_html", "wb+"); 
	if (store == NULL) {
		perror("What!\n");
	}

	search_for_links(output->root, store);

	char *all_urls_buf = allocateBufferForFile(store);

	/* cleanup */
	fclose(store);
	char command[50]; 
	snprintf(command, 50, "rm stored_html");
	system(command); 
	gumbo_destroy_output(&kGumboDefaultOptions, output);
	free(*html_buf);

	return all_urls_buf;
}

