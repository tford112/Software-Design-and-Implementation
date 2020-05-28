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

static void search_for_links(GumboNode* node) {
  if (node->type != GUMBO_NODE_ELEMENT) {
    return;
  }
  GumboAttribute* href;
  if (node->v.element.tag == GUMBO_TAG_A &&
      (href = gumbo_get_attribute(&node->v.element.attributes, "href"))) {
	  printf("%s\n", href->value); 
  }

  GumboVector* children = &node->v.element.children;
  for (unsigned int i = 0; i < children->length; ++i) {
    search_for_links((GumboNode*) children->data[i]);
  }
}


/* The main function is 
 */

int main(int argc, const char** argv) {
						// error checking (command line args) 
	if (argc != 2) {
		perror("Need html\n");
		exit(1);
	}
	FILE *fp = fopen(argv[1], "r"); 
	if (fp == NULL) { 			// error checking file 
		perror("File not found\n");
		exit(2);
	}

	fseek(fp, 0, SEEK_END); 
	int total_bytes = ftell(fp); 
	rewind(fp); 
	char *buf = malloc(sizeof(char) * total_bytes);  // allocate just enough bytes to handle the entire html input 

	if (buf == NULL) {			// error checking buffer{
		perror("Not enough space\n");
		exit(3);
	}
	memset(buf, 0, total_bytes); 
	size_t res = fread(buf, sizeof(char), total_bytes, fp);  
	fclose(fp); 

	GumboOutput* output = gumbo_parse(buf);  // as a reminder, need the buf anyway because gumbo_parse only takes char* inputs 
	search_for_links(output->root);
	gumbo_destroy_output(&kGumboDefaultOptions, output);
	free(buf);
}

