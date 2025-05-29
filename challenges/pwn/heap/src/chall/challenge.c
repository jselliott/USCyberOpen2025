
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>

#define MAX_CHUNKS 15
#define MAX_OPS 15
#define MAX_ALLOC 0x1000

char *chunks[MAX_CHUNKS];
int op_count = 0;

static unsigned char dec64table[] = {

  255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255, /*  0-15 */
  255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255, /* 16-31 */
  255,255,255,255,255,255,255,255,255,255,255, 62,255,255,255, 63, /* 32-47 */
   52, 53, 54, 55, 56, 57, 58, 59, 60, 61,255,255,255,255,255,255, /* 48-63 */
  255,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, /* 64-79 */
   15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25,255,255,255,255,255, /* 80-95 */
  255, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, /* 96-111 */
   41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51,255,255,255,255,255,  /* 112-127*/
   255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
   255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
   255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
   255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
   255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
   255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
   255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
   255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255
};

//Code comes from exim vulnerability cve-2018-6789
//https://github.com/Exim/exim/commit/cf3cd306062a08969c41a1cdd32c6855f1abecf1
int b64decode(const char *code, unsigned char **ptr)
{

int x, y;
//Off by one when handling incorrect strings
unsigned char *result = malloc(3*(strlen(code)/4) + 1);
*ptr = result;

/* Each cycle of the loop handles a quantum of 4 input bytes. For the last
quantum this may decode to 1, 2, or 3 output bytes. */

while ((x = *code++) != 0)
  {
  if (isspace(x)) continue;

  if (x > 127 || (x = dec64table[x]) == 255) goto bad;

  while (isspace(y = *code++)) ;
  if (y == 0 || (y = dec64table[y]) == 255)
    goto bad;

  *result++ = (x << 2) | (y >> 4);

  while (isspace(x = *code++)) ;
  if (x == '=')		/* endmarker, but there should be another */
    {
    while (isspace(x = *code++)) ;
    if (x != '=') goto bad;
    while (isspace(y = *code++)) ;
    if (y != 0) goto bad;
    break;
    }
  else
    {
    if (x > 127 || (x = dec64table[x]) == 255) goto bad;
    *result++ = (y << 4) | (x >> 2);

    while (isspace(y = *code++)) ;
    if (y == '=')
      {
      while (isspace(y = *code++)) ;
      if (y != 0) goto bad;
      break;
      }
    else
      {
      if (y > 127 || (y = dec64table[y]) == 255) goto bad;
      *result++ = (x << 6) | y;
      }
    }
  }
//*result = 0;
return 0;

bad:
	free(*ptr);
	return -1;
}

void menu() {
    puts("1. Allocate and write");
    puts("2. Delete chunk");
    puts("3. Exit");
    printf("> ");
}

void intro() {
    puts("===================================");
    puts("       🧠 Yet Another Heap Game");
    puts("         now with 100% more");
    puts("          🍦 NULL BYTES™!");
    puts("===================================");
}

int custom_atoi(char *s){

    int result = 0;
    int sign = 1;

    // Skip leading whitespace
    while (*s == ' ' || *s == '\t' || *s == '\n') {
        s++;
    }

    // Check for sign
    if (*s == '-') {
        sign = -1;
        s++;
    } else if (*s == '+') {
        s++;
    }

    // Convert characters to integer
    while (*s >= '0' && *s <= '9') {
        result = result * 10 + (*s - '0');
        s++;
    }

    return sign * result;

}

int get_int(){
	char z[10] = {0};
	int a = 0;
	a = read(0,z,9);
	z[a] = 0;
	a = custom_atoi(z);
	return a;
}

void alloc_and_write() {
    /*
    if (op_count++ >= MAX_OPS) {
        puts("Too many operations.");
        exit(0);
    }
    */

    int idx;
    size_t size;
    printf("Index (0-%d): ", MAX_CHUNKS - 1);
    idx = get_int();
    if (idx < 0 || idx >= MAX_CHUNKS || chunks[idx]) {
        puts("Invalid index.");
        return;
    }

    printf("Size (max %d): ", MAX_ALLOC);
    size = get_int();
    if (size == 0 || size > MAX_ALLOC) {
        puts("Invalid size.");
        return;
    }

    char *data = malloc(size+1);
    if (!data) {
        puts("malloc failed");
        exit(1);
    }

    printf("Data: ");
    read(0, data, size);  
    data[size] = 0;

    for(int x = 0; x < size; x++){
	    if (dec64table[data[x]] == 0xff && data[x] != '='){
		    printf("Failed to verify the character %c is base64 decode capable\n", data[x]);
		    exit(0);
	    }
    }

    unsigned char *solution = 0;

    int decoded_len = b64decode(data, &solution);
    if (decoded_len == -1){
	    printf("Failed to base64 decode the string\n");
    }
    else
	    chunks[idx] = solution;

    free(data);
}

void delete_chunk() {
    if (op_count++ >= MAX_OPS) {
        puts("Too many operations.");
        exit(0);
    }

    int idx;
    printf("Index: ");
    idx = get_int();
    if (idx < 0 || idx >= MAX_CHUNKS || !chunks[idx]) {
        puts("Invalid index.");
        return;
    }

    free(chunks[idx]);
    chunks[idx] = NULL;
    puts("Chunk deleted.");
}

int main() {
    setvbuf(stdout, NULL, _IONBF, 0);
    setvbuf(stdin,  NULL, _IONBF, 0);
    intro();

    while (1) {
        menu();
        int choice;
	choice = get_int();
        switch (choice) {
            case 1: alloc_and_write(); break;
            case 2: delete_chunk(); break;
            case 3: puts("Bye!"); exit(0);
            default: puts("Invalid."); break;
        }
    }
    return 0;
}
