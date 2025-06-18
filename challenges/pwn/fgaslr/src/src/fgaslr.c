
#define _GNU_SOURCE

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <stdlib.h>
#include <dlfcn.h>
#include <time.h>
#include <elf.h>

#include "fgaslr.h"
#include "cache.h"
#include "mappings.h"
#include "graph.h"
#include "stats.h"

#include "fgaslr_funcstr.h"
#include "fgaslr_libstr.h"

void *resolve_in_library(const char *function_str, const char *library_str) {

	void *h, *addr;

	h = dlopen(library_str, RTLD_LAZY);

	if (h == NULL) {
		fgaslr_error("Error loading shared library '%s': %s\n", library_str, dlerror());
		exit(-1);
	} else {
		fgaslr_debug("Opened shared library '%s' to resolve '%s'\n", library_str, function_str);
	}

	addr = dlsym(h, function_str);

	if (addr == NULL) {
		fgaslr_error("Error locating '%s' in shared library '%s': %s\n", function_str, library_str, dlerror());
		exit(-1);
	} else {
		fgaslr_debug("Resolved '%s' to %p in '%s'\n", function_str, addr, library_str);
	}

	dlclose(h);

	return addr;

}

char *object_filename(const char *prog_name, const char *function_str) {

	char *filename;

	// TODO: Include the directory, or no?
	filename = malloc(7 + strlen(prog_name) + 1 + strlen(function_str) + 2 + 1);
	sprintf(filename, "%s.o", function_str);
	//sprintf(filename, "%s/%s_%s.o", prog_name, prog_name, function_str);

	return filename;

}

void *generate_random_address() {

	long int r;

	r = ((long int)rand() << 32) | rand();

	r = r % (FGASLR_ADDR_MAX - FGASLR_ADDR_MIN) + FGASLR_ADDR_MIN;

	return (void *)r;

}

unsigned int resolve_symbol(Elf64_Sym *symbol_table, unsigned int symbol_table_size, char *string_table, const char *search) {

	int si;
	Elf64_Sym *symbol;
	char *symbol_name;
	unsigned int symbol_offset;

	for (si=0; si<(symbol_table_size / sizeof(Elf64_Sym)); si++) {

		symbol = symbol_table + si;
		symbol_name = &string_table[symbol->st_name];
		symbol_offset = (unsigned long)symbol->st_value;

		if (strcmp(symbol_name, search) == 0)
			return symbol_offset;

	}

	return -1;

}

void fgaslr_init(const char *parent, struct func *funcs) {

    int fd = open("/dev/urandom", O_RDONLY);
    if (fd < 0) {
        perror("open");
        return;
    }

    uint32_t seed;
    ssize_t bytes_read = read(fd, &seed, sizeof(seed));
    if (bytes_read != sizeof(seed)) {
        perror("read");
        close(fd);
        return;
    }

    close(fd);

    srand(seed);
	
#ifdef ENABLE_GRAPH
	graph_init();
#endif

#ifdef ENABLE_LOADTIME_STATS
	timer_start();
#endif

	fgaslr_resolve(parent, funcs);

#ifdef ENABLE_LOADTIME_STATS
	timer_end();
	loadtime_save();
#endif

#ifdef ENABLE_GRAPH
	graph_fini();
#endif

}
void fgaslr_resolve(const char *parent, struct func *funcs) {

	int c;
	unsigned int i, si, ri, mi;
	unsigned int function_id, library_id;
	const char *function_str, *library_str;
	char *filename;
	int filesize, fd, mapped_text;
	struct stat st;
	void *object, *addr;
	Elf64_Ehdr *elf_header;
	Elf64_Shdr *section_headers;
	Elf64_Rela *relocation;
	Elf64_Sym *symbol_table, *symbol;
	unsigned int section_count, section_offset, section_size, section_type;
	unsigned int num_relocations, symbol_index, relocation_type;
	void *relocation_address;
	unsigned int relocation_value;
	char *section_name, *string_table, *sh_string_table;
	unsigned int symbol_table_offset, symbol_table_size;
	unsigned int symbol_offset, funcs_table_offset;
	struct mapping *mapping, *my_mappings;
	unsigned int my_num_mappings;
	struct func *next_funcs;
#ifdef ENABLE_NAMED_MAPPINGS
	int memfd;
	char *map_name;
#endif

	const char *valid_sections[] = {
		".lot", ".text", ".data", ".bss", ".rodata", ".rodata.str1.1", ".rodata.cst8"
	};

	for (i=0; GET_FUNCID(funcs[i].id) != FUNC_END; i++) {

		function_id = GET_FUNCID(funcs[i].id);
		function_str = funcstr[function_id];
		library_id = GET_LIBID(funcs[i].id);
		library_str = libstr[library_id];

#ifdef ENABLE_GRAPH
		graph_add(parent, function_str);
#endif

		c = cache_search(function_str);

		if (c > -1) {

			fgaslr_debug("'%s' already resolved\n", function_str);
			funcs[i].addr = cache[c].addr;

			continue;

		}

		fgaslr_debug("Resolving '%s'\n", function_str);

		if (library_id == LIB_LIBC) {

			funcs[i].addr = resolve_in_library(function_str, library_str);

		} else if (library_id == LIB_SELF) {

			filename = object_filename(PROG_NAME, function_str);

			stat(filename, &st);
			filesize = st.st_size;

			fd = open(filename, O_RDONLY);

			if (fd < 0) {
				fgaslr_error("Failed to open '%s'\n", filename);
				exit(-1);
			}

			object = mmap(NULL, MALIGN(filesize), PROT_READ, MAP_PRIVATE, fd, 0);

			if (object < 0) {
				fgaslr_error("Failed to map '%s'\n", filename);
				exit(-1);
			}

			elf_header = (Elf64_Ehdr *)object;
			section_headers = (Elf64_Shdr *)(object + elf_header->e_shoff);
			section_count = elf_header->e_shnum;
			sh_string_table = (char *)(object + section_headers[elf_header->e_shstrndx].sh_offset);

			for (si=0; si<section_count; si++) {

				section_name = &sh_string_table[section_headers[si].sh_name];
				section_offset = section_headers[si].sh_offset;
				section_size = section_headers[si].sh_size;
				section_type = section_headers[si].sh_type;

//				fgaslr_debug("Found section '%s' at offset %08x\n", section_name, section_offset);

				if (section_size == 0)
					continue;

				add_mapping(section_name, NULL, si, section_offset, section_size);

				if (section_type == SHT_SYMTAB) {

					symbol_table_offset = section_headers[si].sh_offset;
					symbol_table_size = section_headers[si].sh_size;
					symbol_table = object + symbol_table_offset;

					string_table = object + section_headers[section_headers[si].sh_link].sh_offset;

				}

			}

			addr = generate_random_address();
			mapped_text = 0;

			for (mi=0; mi<(sizeof(valid_sections)/sizeof(char *)); mi++) {

				mapping = get_mapping_by_name(valid_sections[mi]);

				if (mapping == NULL)
					continue;

				if (strcmp(mapping->name, ".text") == 0)
					mapped_text = 1;

#ifdef ENABLE_NAMED_MAPPINGS
				// This probably isn't the greatest, since each mapping will have a
				// shadow copy in kernel space.  That said, this ensures each userspace
				// mapping has a name in /proc/self/maps, which is really helpful for debugging
				// Therefore, only enable if we need to debug the program, disable by default
				map_name = malloc(strlen(function_str) + strlen(mapping->name) + 1);
				sprintf(map_name, "%s%s", function_str, mapping->name);
				memfd = memfd_create(map_name, 0);
				ftruncate(memfd, MALIGN(mapping->size));
				free(map_name);

				mapping->addr = mmap(addr, MALIGN(mapping->size), PROT_READ|PROT_WRITE, MAP_PRIVATE, memfd, 0);
#else
				mapping->addr = mmap(addr, MALIGN(mapping->size), PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS, 0, 0);
#endif

				// If this is the .bss segment, just initialize it to NULL
				// otherwise, copy data from the binary image
				if (strcmp(mapping->name, ".bss") == 0)
					memset(mapping->addr, '\0', mapping->size);
				else
					memcpy(mapping->addr, object + mapping->offset, mapping->size);

				addr += MALIGN(mapping->size);
				fgaslr_debug("section '%s' mapped at %p\n", mapping->name, mapping->addr);

			}

			// only fix up the GOT and .lot if we mapped a .text segment
			if (mapped_text) {

				funcs_table_offset = resolve_symbol(symbol_table, symbol_table_size, string_table, "funcs");
				fgaslr_debug("'funcs' offset is %x\n", funcs_table_offset);

				fgaslr_debug("configuring fake GOT pointer for '%s'\n", function_str);
				mapping = get_mapping_by_name(".lot");
				*(long int *)(mapping->addr) = (long int)mapping->addr + funcs_table_offset;

			// otherwise just unmap the .lot, we don't need it
			} else {

				fgaslr_debug("%s doesn't have a .text, unmapping .lot\n", function_str);
				mapping = get_mapping_by_name(".lot");
				munmap(mapping->addr, MALIGN(mapping->size));

			}

			for (si=0; si<section_count; si++) {

				section_name = &sh_string_table[section_headers[si].sh_name];
				section_offset = section_headers[si].sh_offset;
				section_size = section_headers[si].sh_size;
				section_type = section_headers[si].sh_type;

				if (section_type != SHT_RELA)
					continue;

				if (strcmp(section_name, ".rela.eh_frame") == 0)
					continue;

				num_relocations = section_size / sizeof(Elf64_Rela);

				fgaslr_debug("Processing %u relocations for '%s'\n", num_relocations, function_str);

				for (ri=0; ri<num_relocations; ri++) {

					relocation = (Elf64_Rela *)(object + section_offset + (ri * sizeof(Elf64_Rela)));
					symbol_index = relocation->r_info >> 32;
					relocation_type = relocation->r_info & 0xffffffff;
					relocation_address = get_mapping_by_name(".text")->addr + relocation->r_offset;

					fgaslr_debug("Relocation entry %d: offset=%lx, info=%lx, type=%x, addend=%ld\n", ri, relocation->r_offset, relocation->r_info, relocation_type, relocation->r_addend);

					switch (relocation_type) {

					case R_X86_64_REX_GOTPCRELX:

						relocation_value =
							get_mapping_by_name(".lot")->addr
							- ( get_mapping_by_name(".text")->addr + relocation->r_offset )
							- 4;

						*(unsigned int *)relocation_address = (unsigned int)relocation_value;
						fgaslr_debug("R_X86_64_REX_GOTPCRELX: %p -> %08x\n", (void *)relocation_address, relocation_value);

						break;

					case R_X86_64_PC32:

						symbol = symbol_table + symbol_index;
//						symbol_name = object + section_headers[symbol->st_shndx].sh_offset + symbol->st_name;

						mapping = get_mapping_by_index(symbol->st_shndx);

/*
						// TODO: UNTESTED
						if (strncmp(symbol_name, ".L.str", 6) == 0) {

							relocation_value =
								( (get_mapping_by_name(mappings, section_count, ".rodata.str1.1"))->offset + symbol->st_value )
								- ( (get_mapping_by_name(mappings, section_count, ".text"))->offset + relocation->r_offset )
								- relocation->r_addend;

						// TODO: UNTESTED
						} else if (strncmp(symbol_name, ".LCPI0_", 7) == 0) {

							relocation_value =
								( (get_mapping_by_name(mappings, section_count, ".rodata.cst8"))->offset + symbol->st_value )
								- ( (get_mapping_by_name(mappings, section_count, ".text"))->offset + relocation->r_offset )
								- relocation->r_addend;
						}
*/
						relocation_value =
							( mapping->addr + symbol->st_value )
							- ( get_mapping_by_name(".text")->addr + relocation->r_offset )
							+ relocation->r_addend;

						*(unsigned int *)relocation_address = (unsigned int)relocation_value;
						fgaslr_debug("R_X86_64_PC32: %p -> %08x\n", (void *)relocation_address, relocation_value);

						break;

					default:
						fgaslr_error("Unknown relocation type: %u\n", relocation_type);

					}

				}

			}

			fgaslr_debug("Locating symbol '%s'\n", function_str);
			symbol_offset = resolve_symbol(symbol_table, symbol_table_size, string_table, function_str);

			fgaslr_debug("Found symbol '%s' at offset %08x\n", function_str, symbol_offset);

			// Search for the first valid section that was mapped, and assume the symbol is
			// a part of that mapping.  This is honestly pretty sketchy, might not work
			// in all cases, but seems to be stable for now.
			for (mi=1; mi<(sizeof(valid_sections)/sizeof(char *)); mi++) {

				mapping = get_mapping_by_name(valid_sections[mi]);
				if (mapping == NULL)
					continue;

				funcs[i].addr = mapping->addr + symbol_offset;
				break;

			}

			fgaslr_debug("Adding %s:%p to the cache\n", function_str, funcs[i].addr);
			cache_add(function_str, funcs[i].addr);

			// iff we have a .lot and a .text, recursively resolve functions
			if (mapped_text) {

				fgaslr_debug("Recursively resolving functions in '%s'\n", function_str);

				next_funcs = (struct func *)(get_mapping_by_name(".lot")->addr + funcs_table_offset);

				my_mappings = mappings;
				my_num_mappings = num_mappings;

				mappings = NULL;
				num_mappings = 0;

				fgaslr_resolve(function_str, next_funcs);

				mappings = my_mappings;
				num_mappings = my_num_mappings;

				fgaslr_debug("Finished recursively resolving functions in '%s'\n", function_str);

			}

			mapping = get_mapping_by_name(".lot");
			if (mapping != NULL)
				mprotect(mapping->addr, MALIGN(mapping->size), PROT_READ|PROT_WRITE);

			mapping = get_mapping_by_name(".text");
			if (mapping != NULL)
				mprotect(mapping->addr, MALIGN(mapping->size), PROT_READ|PROT_EXEC);

			mapping = get_mapping_by_name(".data");
			if (mapping != NULL)
				mprotect(mapping->addr, MALIGN(mapping->size), PROT_READ|PROT_WRITE);

			mapping = get_mapping_by_name(".bss");
			if (mapping != NULL)
				mprotect(mapping->addr, MALIGN(mapping->size), PROT_READ|PROT_WRITE);

			mapping = get_mapping_by_name(".rodata");
			if (mapping != NULL)
				mprotect(mapping->addr, MALIGN(mapping->size), PROT_READ);

			free_mappings();
			munmap(object, MALIGN(filesize));
			close(fd);
			free(filename);

		} else {

			fgaslr_error("Unknown library '%s' (%u)\n", library_str, library_id);

		}

	}

}
