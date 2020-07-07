// Copyright 2013 Google Inc. All Rights Reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
// Author: jdtang@google.com (Jonathan Tang)
//
// Retrieves the title of a page.

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

int main(int argc, const char** argv) {
	if (argc != 2) {
		perror("Need html\n");
		exit(1);
	}
	FILE *fp = fopen(argv[1], "r"); 
	if (fp == NULL) {
		perror("File not found\n");
		exit(2);
	}
	fseek(fp, 0, SEEK_END); 
	int total_bytes = ftell(fp); 
	rewind(fp); 
	char *buf = malloc(sizeof(char) * total_bytes); 
	if (buf == NULL) {
		perror("Not enough space\n");
		exit(3);
	}
	memset(buf, 0, total_bytes); 
	size_t res = fread(buf, sizeof(char), total_bytes, fp); 
	fclose(fp); 

	GumboOutput* output = gumbo_parse(buf);
	search_for_links(output->root);
//	gumbo_destroy_output(&kGumboDefaultOptions, output);
	free(buf);
}

