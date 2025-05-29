
#include <string.h>

#include "cache.h"

struct cache_entry cache[MAX_FUNCS];

int cache_search(const char *name) {

	int i;

	for (i=0; i<MAX_FUNCS && cache[i].name; i++) {
		if (strcmp(cache[i].name, name) == 0)
			return i;
	}

	return -1;

}

void cache_add(const char *name, void *p) {

	int i;

	for (i=0; i<MAX_FUNCS && cache[i].name; i++);

	cache[i].name = strdup(name);
	cache[i].addr = p;

}

void cache_shred() {

	int i;

	for (i=0; i<MAX_FUNCS; i++) {
		cache[i] = (struct cache_entry) {NULL, NULL};
	}

}
