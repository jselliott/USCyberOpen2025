/*
 */
#include "uhttpd.h"
#include "cgi_utils.h"


int safe_strcmp(const char *s1, const char *s2) 
{
    char emptyStr = '\0';
    char *str1 = (char *) s1;
    char *str2 = (char *) s2;
    
    if (str1 == NULL)
    {
        str1 = &emptyStr;
    }
    if (str2 == NULL)
    {
        str2 = &emptyStr;
    }
    
    return strcmp(str1, str2);
}

int safe_strlen(const char *s1) 
{
    char emptyStr = '\0';
    char *str1 = (char *) s1;

    if (str1 == NULL)
    {
        str1 = &emptyStr;
    }

    return strlen(str1);
}

char *gui_strstr(const char *s1, const char *s2)
{
    char *str1 = (char *) s1;
    char *str2 = (char *) s2;
    
    if (str1 == NULL)
    {
        return NULL;
    }
    if (str2 == NULL)
    {
        return NULL;
    }
    
    return strstr(str1, str2);
}


void gui_hex2asc(unsigned char *hexStr, unsigned int hexLen, char *ascStr, unsigned int ascLen)
{
    int i;
    unsigned char uc;
    for (i=0;i<hexLen;i++){
        if (i > ascLen*2)
            break;
        
        uc = hexStr[i];
        if (uc >= 0x30  && uc <= 0x39)
            uc -= 0x30;
        else if (uc >= 0x41 && uc <= 0x46)
            uc -= 0x37;
        else if (uc >= 0x61 && uc <= 0x66)
            uc -= 0x57;
        else
            break;
        
        if (i%2 == 1)
            ascStr[(i-1)/2] = ascStr[(i-1)/2] | uc;
        else
            ascStr[i/2] = uc << 4;
    }
}
void gui_asc2hex(char *ascStr, unsigned int ascLen, unsigned char *hexStr, unsigned int hexLen)
{
    int i;
    unsigned char uc;
    for (i=0;i<ascLen;i++){
        if (i*2 >= hexLen) break;
            uc = (ascStr[i] & 0xF0)  >> 4;
        
        if (uc < 0x0A) 
            uc += 0x30;
        else 
            uc+=0x37;
        
        hexStr[i*2] = uc;
        uc = ascStr[i] & 0x0F;
        if (uc < 0x0A) 
            uc += 0x30;
        else 
            uc += 0x37;
        
        if (i*2+1 >= hexLen) break;
        hexStr[i*2+1] = uc;
    }
}

 
/**
  @brief    find a char string in a char memory block
  @param    block   Char block to be searched
  @param    bsize   Size of the char block to be searched
  @param    pattern Character string to look for
  @return   char pointer to the pattern in block, or NULL.

  Find a char pattern in a char memory block. This does not support
  regular expressions. The functionality is strictly similar to
  strstr(), except that the searched block is allowed to contain NULL
  characters.
 */
/*--------------------------------------------------------------------------*/

char * gui_memstr(char * block, int bsize, char * pattern)
{
    int     found ;
    char *  where ;
    char *  start ;

    found = 0 ;
    start = block ;
    while (!found) {
        where = (char*)memchr(start, (int)pattern[0], (size_t)bsize - (size_t)(start - block));
        if (where==NULL) {
            found++ ;
        } else {
            if (memcmp(where, pattern, strlen(pattern))==0) {
                found++;
            }
        }
        start=where+1 ;
    }
    return where ;
}

/*
 * Reverse memchr()
 * Find the last occurrence of 'c' in the buffer 's' of size 'n'.
 */
void *gui_memrchr(const void *s, int c, size_t n)
{
  const unsigned char *start=s,*end=s;

  end+=n-1;

  while(end>=start)
    {
      if(*end==c)
        return (void *)end;
          else
        end--;
    }

    return NULL;
}


/*
 * split special string, save to string array.
*/
void gui_split(char *src, const char *separator, char **dest, int *num)
{
    char *pNext;
    int count = 0;
   
    pNext = strtok(src,separator);
   
    while(pNext != NULL)
    {
        *dest++ = pNext;
        ++count;
        pNext = strtok(NULL,separator);
    }

    *num = count;
}


/*
 */
void gui_trim(char *s)
{
    int len = strlen(s);
    int lws;

    /* trim trailing whitespace */
    while (len && isspace(s[len-1]))
        --len;

    /* trim leading whitespace */
    if (len) {
        lws = strspn(s, " \n\r\t\v");
        if (lws) {
            len -= lws;
            memmove(s, s + lws, len);
        }
    }
    s[len] = '\0';
}

float gui_distime(struct timeval tv1, struct timeval tv2)
{
    unsigned int sec1 = tv1.tv_sec;
    unsigned int usec1 = tv1.tv_usec;
    
    unsigned int sec2 = tv2.tv_sec;
    unsigned int usec2 = tv2.tv_usec;
    
    //gui_debug("sec1=%u, usec1=%u, sec2=%u, usec2=%u", sec1, usec1, sec2, usec2);
    float f_usec1 = (usec1 * 1.0) / 1000000.0;
    float f_usec2 = (usec2 * 1.0) / 1000000.0;
    
    float dis = (float)(sec2 - sec1);
    //gui_debug("dis=%f", dis);
    dis += f_usec2 - f_usec1;
    
    //gui_debug("f_usec1=%f, f_usec2=%f, dis=%f", f_usec1, f_usec2, dis);
    return dis;
}


/* Encode the output value base on microsoft standard*/
char* GUI_SpeciCharEncode(char *s, int len)
{
    int c;
    int n = 0;
    char t[4097]={0};
    char *p;
    
    p=s;
    memset(t,0,sizeof(t));
    
    if (s == NULL) {
        gui_debug("The transfer object is null");
        return s;
    }
    while ((c = *p++)&& n < 4096) {        
        if (!strchr("\n\r<>\"'%;)(&+|,/\\", c)) {
            t[n++] = c;
        } else if (n < 4096-5) {
            t[n++] = '&';
            t[n++] = '#';

            if(strchr("|",c)){
                t[n++] = (c/100) + '0';
                t[n++] = ((c%100)/10) + '0';
                t[n++] = ((c%100)%10) + '0';
            }else{
                t[n++] = (c/10) + '0';
                t[n++] = (c%10) + '0';
            }
            t[n++] = ';';
        } else {
            gui_debug("The Array size overflow Exception");
            return s;;
        }
    }
    t[n] = '\0';
    if (n <= len && n < 4096)
    {
        memset(s,0,len);
        strncpy(s, t,  n);
    }
    else
        gui_debug("The Array size overflow Exception");
    return s;
}

//use stat get filesize
unsigned long get_file_size(const char *path)  
{  
    unsigned long filesize = -1;      
    struct stat statbuff;  
    if(stat(path, &statbuff) < 0){  
        return filesize;  
    }else{  
        filesize = statbuff.st_size;  
    }  
    return filesize;  
} 
