
#ifndef STATS_H
#define STATS_H

#define LOADTIME_FILE	"./data/loadtime.csv"
#define RUNTIME_FILE	"./data/runtime.csv"

void timer_start();
void timer_end();

long unsigned int get_elapsed_ns();

void loadtime_save();
void runtime_save();

#endif