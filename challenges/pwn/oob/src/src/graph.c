
#include <stdio.h>
#include <stdlib.h>

#include "graph.h"

FILE *gfp;

void graph_init() {

	gfp = fopen(GRAPH_FILE, "w");

	if (!gfp) {
		printf("Failed to open graph file '%s'.\n", GRAPH_FILE);
		exit(-1);
	}

	fprintf(gfp, "var dot = \"dinetwork {node[shape=circle]; edge [color=gray]; ");

}

void graph_fini() {

	fprintf(gfp, "}\";");

	fclose(gfp);

}

void graph_add(const char *parent, const char *child) {

	fprintf(gfp, "%s -> %s ; ", parent, child);
	fprintf(gfp, "%s [color=#%06x] ; ", parent, str_color(parent));
	fprintf(gfp, "%s [color=#%06x] ; ", child, str_color(child));

}

int str_color(const char *s) {

	int i, color = 0xc0ffee;

	for (i=0; s[i]!='\0'; i++) {

		color *= s[i];
		color &= 0xffffff;

	}

	return color;

}