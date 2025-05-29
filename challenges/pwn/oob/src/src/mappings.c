
#include <string.h>
#include <stdlib.h>

#include "mappings.h"

struct mapping *mappings = NULL;

unsigned int num_mappings = 0;

struct mapping *get_mapping_by_name(const char *name) {

	int i;

	for (i=0; i<num_mappings; i++) {

		if (strcmp(mappings[i].name, name) == 0)
			return &mappings[i];

	}

	return NULL;

}

struct mapping *get_mapping_by_index(unsigned int index) {

	int i;

	for (i=0; i<num_mappings; i++) {

		if (mappings[i].index == index)
			return &mappings[i];

	}

	return NULL;

}

struct mapping *get_mapping(unsigned long search) {

	// this is quite dubious, but will work in 99.99% of situations
	// it cleans up the code quite a bit, so we'll try it :D

	// most programs will have <= 0x1000 sections
	// most pointers will be > 0x1000

	if (search <= 0x1000) {

		return get_mapping_by_index((unsigned int)search);

	} else {

		return get_mapping_by_name((const char *)search);

	}

}

void add_mapping(const char *name, void *addr, unsigned int index, unsigned int offset, unsigned int size) {

	mappings = realloc(mappings, (num_mappings + 1) * sizeof(struct mapping));

	mappings[num_mappings].name = strdup(name);
	mappings[num_mappings].addr = addr;
	mappings[num_mappings].index = index;
	mappings[num_mappings].offset = offset;
	mappings[num_mappings].size = size;

	num_mappings += 1;

}

void free_mappings() {

	int i;

	for (i=0; i<num_mappings; i++)
		free(mappings[i].name);

	free(mappings);

	num_mappings = 0;
	mappings = NULL;

}
