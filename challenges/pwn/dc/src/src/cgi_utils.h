#ifndef CGI_UTILS_H
#define CGI_UTILS_H

#include <stdarg.h>
#include <fcntl.h>
#include <pwd.h>
#include <sys/stat.h>


int safe_strcmp(const char *s1, const char *s2);
int safe_strlen(const char *s1) ;
char *gui_strstr(const char *s1, const char *s2);

void gui_hex2asc(unsigned char *hexStr, unsigned int hexLen, char *ascStr, unsigned int ascLen);
void gui_asc2hex(char *ascStr, unsigned int ascLen, unsigned char *hexStr, unsigned int hexLen);
char *gui_memstr(char * block, int bsize, char * pattern);
void *gui_memrchr(const void *s, int c, size_t n);


void gui_split(char *src, const char *separator, char **dest, int *num);
void gui_trim(char *s);

float gui_distime(struct timeval tv1, struct timeval tv2);
char* GUI_SpeciCharEncode(char *s, int len);
unsigned long get_file_size(const char *path);
#endif
