#ifndef CACHE_H
#define CACHE_H

#define MAX_FUNCS 1024

struct cache_entry {
	char *name;
	void *addr;
};

extern struct cache_entry cache[];

int cache_search(const char *name);
void cache_add(const char *name, void *p);
void cache_shred();

#endif