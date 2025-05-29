
#ifndef GRAPH_H
#define GRAPH_H

#define GRAPH_FILE "./data/graph_data.js"

void graph_init();
void graph_fini();
void graph_add(const char *parent, const char *child);
int str_color(const char *s);

#endif