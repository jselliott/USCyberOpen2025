
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "stats.h"

struct timespec start, end;

void timer_start() {

	clock_gettime(CLOCK_MONOTONIC, &start);

}

void timer_end() {

	clock_gettime(CLOCK_MONOTONIC, &end);

}

long unsigned int get_elapsed_ns() {

	long unsigned int diff;

	diff = (end.tv_sec - start.tv_sec) * 1000000000L;
	diff += end.tv_nsec - start.tv_nsec;

	return diff;

}

void loadtime_save() {

	FILE *fp;

	fp = fopen(LOADTIME_FILE, "a");

	if (!fp) {
		printf("Failed to open loadtime file '%s'\n", LOADTIME_FILE);
		exit(-1);
	}

	fprintf(fp, "%lu\n", get_elapsed_ns());

	fclose(fp);

}

void runtime_save() {

	FILE *fp;

	fp = fopen(RUNTIME_FILE, "a");

	if (!fp) {
		printf("Failed to open runtime file '%s'\n", RUNTIME_FILE);
		exit(-1);
	}

	fprintf(fp, "%lu\n", get_elapsed_ns());

	fclose(fp);

}

