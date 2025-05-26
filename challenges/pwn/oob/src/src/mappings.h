#ifndef MAPPINGS_H
#define MAPPINGS_H

struct mapping {
	char *name;
	void *addr;
	unsigned int index;
	unsigned int offset;
	unsigned int size;
};

extern struct mapping *mappings;

extern unsigned int num_mappings;

struct mapping *get_mapping_by_name(const char *name);
struct mapping *get_mapping_by_index(unsigned int index);
struct mapping *get_mapping(unsigned long search);
void add_mapping(const char *name, void *addr, unsigned int index, unsigned int offset, unsigned int size);
void free_mappings();

#endif