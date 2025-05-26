/*
 * uhttpd - Tiny single-threaded httpd - Utility functions
 *
 *   Copyright (C) 2010 Jo-Philipp Wich <xm@subsignal.org>
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 */

#define _XOPEN_SOURCE 500   /* crypt() */
#define _BSD_SOURCE         /* strcasecmp(), strncasecmp() */

#include "uhttpd.h"
#include "uhttpd-utils.h"
#include <time.h>

#ifdef HAVE_TLS
#include "uhttpd-tls.h"
#endif

#define AEI_REVJ 1
#define BUFLEN_1024 1024
#define BUFLEN_64 64
#define BUFLEN_256 256
#define BUFLEN_128 128
#define UPLOAD_FILE_PREFIX "/tmp/"

#include "cgi_utils.h"

#ifdef AEI_UHTTPD_REALM_AUTH
#ifdef CONFIG_CUSTOM_REALM
#include "custom_realm.h"
#else
URLMap2Realm uhttp_realm[] = {
    {
        .protol = "http",
        .url_str = "*",
        .port_range = {.low=80, .high = 80},
        .realm = "default"
    },
};
#endif

#define UHTTPD_REALM_COUNT (sizeof(uhttp_realm)/sizeof(uhttp_realm[0]))
#endif
static char *uh_index_files[] = {
    "login.html",
    "login.htm",
    "default.html",
    "default.htm"
};


const char * sa_straddr(void *sa)
{
    static char str[INET6_ADDRSTRLEN];
    struct sockaddr_in *v4 = (struct sockaddr_in *)sa;
    struct sockaddr_in6 *v6 = (struct sockaddr_in6 *)sa;

    if( v4->sin_family == AF_INET )
        return inet_ntop(AF_INET, &(v4->sin_addr), str, sizeof(str));
    else
        return inet_ntop(AF_INET6, &(v6->sin6_addr), str, sizeof(str));
}

#ifdef AEI_REVJ
const char *get_cli_addr(void *sa)
{
return inet_ntoa(((struct sockaddr_in *)sa)->sin_addr);
}
#endif

const char * sa_strport(void *sa)
{
    static char str[6];
    snprintf(str, sizeof(str), "%i", sa_port(sa));
    return str;
}

int sa_port(void *sa)
{
    return ntohs(((struct sockaddr_in6 *)sa)->sin6_port);
}

int sa_rfc1918(void *sa)
{
    struct sockaddr_in *v4 = (struct sockaddr_in *)sa;
    unsigned long a = htonl(v4->sin_addr.s_addr);

    if( v4->sin_family == AF_INET )
    {
        return ((a >= 0x0A000000) && (a <= 0x0AFFFFFF)) ||
               ((a >= 0xAC100000) && (a <= 0xAC1FFFFF)) ||
               ((a >= 0xC0A80000) && (a <= 0xC0A8FFFF));
    }

    return 0;
}

/* Simple strstr() like function that takes len arguments for both haystack and needle. */
char *strfind(char *haystack, int hslen, const char *needle, int ndlen)
{
    int match = 0;
    int i, j;

    for( i = 0; i < hslen; i++ )
    {
        if( haystack[i] == needle[0] )
        {
            match = ((ndlen == 1) || ((i + ndlen) <= hslen));

            for( j = 1; (j < ndlen) && ((i + j) < hslen); j++ )
            {
                if( haystack[i+j] != needle[j] )
                {
                    match = 0;
                    break;
                }
            }

            if( match )
                return &haystack[i];
        }
    }

    return NULL;
}

#ifdef AEI_REVJ
static char *get_http_header(char *buffer, int buflen, const char *needle)
{
    char *headers = NULL;
    char *hdrname = NULL;
    char *hdrdata = NULL;
    
    char bufheader[1024] = {0};
    char *bufptr = &bufheader[0];

    int i;
    
    memcpy(bufptr, buffer, buflen);
    bufheader[buflen] = '\0';

    //terminate initial header line
    if( (headers = strfind(bufptr, buflen, "\r\n", 2)) != NULL )
    {
        bufptr[buflen-1] = 0;

        *headers++ = 0;
        *headers++ = 0;

        // process header fields
        for( i = (int)(headers - bufptr); i < buflen; i++ )
        {
            // found eol and have name + value, push out header tuple
            if( hdrname && hdrdata && (bufptr[i] == '\r' || bufptr[i] == '\n') )
            {
                bufptr[i] = 0;
                // store
                if(strcmp(hdrname, needle) == 0)
                {
                    return hdrdata;
                }
                hdrname = hdrdata = NULL;
            }
            // have name but no value and found a colon, start of value
            else if( hdrname && !hdrdata && ((i+2) < buflen) &&
                (bufptr[i] == ':') && (bufptr[i+1] == ' ')
            ) {
                bufptr[i] = 0;
                hdrdata = &bufptr[i+2];
            }
            // have no name and found [A-Za-z], start of name
            else if( !hdrname && isalpha(bufptr[i]) )
            {
                hdrname = &bufptr[i];
            }
        }   
    }

    return NULL;
}
#endif

/* interruptable select() */
int select_intr(int n, fd_set *r, fd_set *w, fd_set *e, struct timeval *t)
{
    int rv;
    sigset_t ssn, sso;

    /* unblock SIGCHLD */
    sigemptyset(&ssn);
    sigaddset(&ssn, SIGCHLD);
    sigaddset(&ssn, SIGPIPE);
    sigprocmask(SIG_UNBLOCK, &ssn, &sso);

    rv = select(n, r, w, e, t);

    /* restore signal mask */
    sigprocmask(SIG_SETMASK, &sso, NULL);

    return rv;
}


int uh_tcp_send(struct client *cl, const char *buf, int len)
{
    fd_set writer;
    struct timeval timeout;

    gui_debug("cl_addr:%p, cl_ser_addr:%p, cl_ser_conf_addr:%p\n", cl,12344);
    if(NULL == cl)//; || NULL == cl->server || NULL == cl->server->conf)
        return -1;
    gui_debug("sokid:%d\n", cl->socket);
    FD_ZERO(&writer);
    FD_SET(cl->socket, &writer);

    //timeout.tv_sec = cl->server->conf->network_timeout;
    timeout.tv_usec = 0;

    if( 1)//select(cl->socket + 1, NULL, &writer, NULL, &timeout) > 0 )
    {
#ifdef HAVE_TLS
        if( cl->tls )
            return cl->server->conf->tls_send(cl, (void *)buf, len);
        else
#endif
            //return send(cl->socket, buf, len, 0);
            return write(1, buf, len);
    }

    return -1;
}

#ifdef AEI_REVJ
int uh_tcp_recv_header(struct client *cl, char *buf, int len)
{
    int sz = 0;
    int rsz = 0;

    fd_set reader;
    struct timeval timeout;

    if(NULL == cl)// || NULL == cl->server || NULL == cl->server->conf)
        return -1;
    FD_ZERO(&reader);
    FD_SET(cl->socket, &reader);

    //timeout.tv_sec  = cl->server->conf->network_timeout;
    timeout.tv_usec = 0;

    while(1)
    {
        if( 1)//select(cl->socket + 1, &reader, NULL, NULL, &timeout) > 0 )
        {
#ifdef HAVE_TLS
            if( cl->tls )
                rsz = cl->server->conf->tls_recv(cl, (void *)&buf[sz], len);
            else
#endif
                //rsz = recv(cl->socket, (void *)&buf[sz], len, 0);
	    	rsz = read(0, (void *)&buf[sz], len);
            
            //gui_debug("sz=%d, rsz=%d", sz, rsz);
            if( (sz == 0) || (rsz > 0) )    sz += rsz;
            if (sz > (UH_LIMIT_MSGHEAD-1))
            {
                buf[UH_LIMIT_MSGHEAD-1] = '\0';
                break;
            }
            else if (rsz <= 0)
            {
                buf[sz] = '\0';
                break;
            }
            
            buf[sz] = '\0';
        
            //gui_debug("sz=%d, buf=%s", sz, buf);
            char *idxptr;
            if ((idxptr = strfind(buf, sz, "\r\n\r\n", 4)) != NULL)
            {
                char *pcontent_type = NULL;
                if ((pcontent_type = strfind(buf, sz, "multipart/form-data", strlen("multipart/form-data"))) != NULL)
                {
                    char *pfilename = NULL;
                    if ((pfilename = strfind(buf, sz, "filename=", strlen("filename="))) != NULL)
                        break;
                }
                else    
                    break;
            }

            if (sz > UH_LIMIT_MSGHEAD/10)
                break;
        }
        else if( sz == 0 )
        {
            sz = -1;
            break;
        }
        else
            break;
    }
    
    /* store received data in peek buffer */
    if( sz > 0 )
    {
        cl->peeklen = sz;
        memcpy(cl->peekbuf, buf, sz);
    }

    return sz;
}

int uh_tcp_recv_content(struct client *cl, char *buf, int headerlen, int contentlen)
{
    int sz = 0;
    int rsz = 0;

    fd_set reader;
    struct timeval timeout;
    
    //gui_debug("cl->peeklen=%d, headerlen=%d, contentlen=%d", cl->peeklen, headerlen, contentlen);
    
    if (cl->peeklen > (headerlen + contentlen)) return 0;       //error
    if (cl->peeklen == (headerlen + contentlen))
    {
        //memcpy(buf, cl->peekbuf, sz);
        memmove(cl->peekbuf, &cl->peekbuf[headerlen], contentlen);
        cl->peekbuf[(contentlen>(UH_LIMIT_MSGHEAD-1))?(UH_LIMIT_MSGHEAD-1):contentlen] = '\0';
        return 0;
    }
    
    sz = cl->peeklen;

    /* caller wants more */
    //if( len > 0 )
    {
        FD_ZERO(&reader);
        FD_SET(cl->socket, &reader);

        //timeout.tv_sec  = cl->server->conf->network_timeout;
        timeout.tv_usec = 0;

        while(1)
        {
            if( 1)//select(cl->socket + 1, &reader, NULL, NULL, &timeout) > 0 )
            {
#ifdef HAVE_TLS
                if( cl->tls )
                    rsz = cl->server->conf->tls_recv(cl, (void *)&buf[sz], contentlen);
                else
#endif
                    //rsz = recv(cl->socket, (void *)&buf[sz], contentlen, 0);
		    rsz = read(0, (void *)&buf[sz], contentlen);
                
                //gui_debug("sz=%d, rsz=%d", sz, rsz);
                if( (sz == 0) || (rsz > 0) )    sz += rsz;
                if (sz > (UH_LIMIT_MSGHEAD-1))
                {
                    buf[UH_LIMIT_MSGHEAD-1] = '\0';
                    break;
                }
                else if (rsz <= 0)
                {
                    buf[sz] = '\0';
                    break;
                }
                
                
                buf[sz] = '\0';

                //gui_debug("rsz=%d, cl->peeklen=%d, sz=%d, headerlen=%d, contentlen=%d", rsz, cl->peeklen, sz, headerlen, contentlen);

                if (sz == (headerlen + contentlen))
                {
                    cl->peeklen = contentlen;
                    memcpy(cl->peekbuf, &buf[headerlen], contentlen);
                    cl->peekbuf[(contentlen>(UH_LIMIT_MSGHEAD-1))?(UH_LIMIT_MSGHEAD-1):contentlen] = '\0';
                    break;
                }
            }
            else if( sz == 0 )
            {
                //gui_debug("sz=%d", sz);
                sz = -1;
                break;
            }
            else
            {
                //gui_debug("sz=%d", sz);
                break;
            }
        }
    }
    //gui_debug("cl->peekbuf=%s", cl->peekbuf);
    return sz;
}


void uh_get_postdata_withupload(char *buf, int buflen, char *postdata, char *boundary)
{
    char str[UH_LIMIT_MSGHEAD] = {0};
    char *pstr = &str[0];
    strncpy(str, buf, buflen);
    
    char *p = NULL;
    if ((p = gui_strstr(str, "filename=")) != NULL)
    {
        str[p-&str[0]] = '\0';
    }

#if 1
    int paramnum = 0;
    //gui_debug("pstr=%s", pstr);
    while ((p = gui_strstr(pstr, boundary)) != NULL)
    {
        char data[BUFLEN_1024] = {0};
        char *pdata = &data[0];
        
        strncpy(data, pstr, (int)(p-pstr));
        gui_debug("data=%s", data);
        pstr = p+strlen(boundary);
        
        char *q = NULL;
        q = gui_strstr(data, "\r\n\r\n");
        q += 4;
        gui_debug("value=%s", q);
        
        char fieldname[BUFLEN_64] = {0};
        char buf0[BUFLEN_1024] = {0};
        memcpy(buf0, pdata, (int)(q-pdata));
        char *pfieldname1 = NULL;
        char *pfieldname2 = NULL;
        pfieldname1 = strrchr(buf0, '=');
        pfieldname2 = strrchr(buf0, '\"');
        memcpy(fieldname, &data[(int)(pfieldname1-pdata)+2], (int)(pfieldname2-pfieldname1)-2);
        gui_debug("fieldname=%s", fieldname);
        
        if (strlen(fieldname) > 0)
        {
            if (paramnum > 0)
                strcat(postdata, "&");
                
            strcat(postdata, fieldname);
            strcat(postdata, "=");
            strcat(postdata, q);
            
            paramnum ++;
        }
                
        //gui_debug("pstr=%s", pstr);
    }

#endif  
}


int uh_tcp_recv_file(struct client *cl, int headerlen, int contentlen, char *boundary, char *uploadFileName)
{
    int sz = 0;
    int rsz = 0;
    FILE *fp;
    int alreadyopen = 0;
    unsigned char hexboundary[BUFLEN_256] = {0x00};
    char postdata[BUFLEN_1024] = {0};
    char buf[UH_LIMIT_MSGHEAD*2] = {0};

    fd_set reader;
    struct timeval timeout;
    
    //gui_debug("cl->peeklen=%d, headerlen=%d, contentlen=%d", cl->peeklen, headerlen, contentlen);
    
    if (cl->peeklen > (headerlen + contentlen)) return 0;   //error
    gui_asc2hex(boundary, strlen(boundary), hexboundary, strlen(boundary)*2+1);
    if (cl->peeklen == (headerlen + contentlen))
    {
        int realContentLen = 0;
        realContentLen = contentlen - strlen(boundary);
        memcpy(buf, &cl->peekbuf[headerlen+strlen(boundary)], realContentLen);

        uh_get_postdata_withupload(buf, realContentLen, postdata, boundary);
        // it is a bug here. use &fp instead fp
        //uh_save_upload_file(&fp, cl, &alreadyopen, buf, realContentLen, boundary, hexboundary, postdata, uploadFileName);
        
        if (fp)
            fclose(fp);
        return 0;
    }

    char buf2[UH_LIMIT_MSGHEAD*2] = {0};
    int idxbuf2 = 0;
        
    //gui_debug("cl->peeklen=%d, headerlen=%d", cl->peeklen, headerlen);
    sz = cl->peeklen;
    memmove(cl->peekbuf, &cl->peekbuf[headerlen], cl->peeklen-headerlen);
    memcpy(buf, &cl->peekbuf[strlen(boundary)], cl->peeklen-headerlen-strlen(boundary));
    
    uh_get_postdata_withupload(buf, cl->peeklen-headerlen-strlen(boundary), postdata, boundary);
    if (1)//uh_save_upload_file(&fp, cl, &alreadyopen, buf, cl->peeklen-headerlen-strlen(boundary), boundary, hexboundary, postdata, uploadFileName) == 1)
        return sz;

    /* caller wants more */
    //if( len > 0 )
    {
        FD_ZERO(&reader);
        FD_SET(cl->socket, &reader);

        //timeout.tv_sec  = cl->server->conf->network_timeout;
        timeout.tv_usec = 0;
        
        
        while(1)
        {
            if( select(cl->socket + 1, &reader, NULL, NULL, &timeout) > 0 )
            {
#ifdef HAVE_TLS
                if( cl->tls )
                    rsz = cl->server->conf->tls_recv(cl, (void *)&buf2[idxbuf2], UH_LIMIT_MSGHEAD-1);
                else
#endif
                    //rsz = recv(cl->socket, (void *)&buf2[idxbuf2], UH_LIMIT_MSGHEAD-1, 0);
	    	    rsz = read(0, (void *)&buf[sz], UH_LIMIT_MSGHEAD-1);
                
                //gui_debug("idxbuf2=%d, sz=%d, rsz=%d", idxbuf2, sz, rsz);
                if( (sz == 0) || (rsz > 0) )    sz += rsz;
                if (rsz <= 0)
                {
                    break;
                }               

                memset(buf, 0, sizeof(buf));
                memcpy(buf, buf2, rsz);
                char *idxptr = NULL;
                int buflen = 0;
                
#if 1
                unsigned char hexbuf[UH_LIMIT_MSGHEAD*2] = {0x00};
                gui_asc2hex(buf, rsz, hexbuf, rsz*2+1);
                idxptr = gui_memrchr(hexbuf, 13, strlen((char *)hexbuf));
                if (idxptr)
                {
                    hexbuf[(int)((unsigned char*)idxptr-&hexbuf[0])] = '\0';
                    char ascbuf[UH_LIMIT_MSGHEAD] = {0};
                    gui_hex2asc(hexbuf, strlen((char *)hexbuf), ascbuf, strlen((char *)hexbuf)/2);
                                        
                    buflen = strlen((char *)hexbuf)/2;
                    if (1)//uh_save_upload_file(&fp, cl, &alreadyopen, ascbuf, buflen, boundary, hexboundary, postdata, uploadFileName) == 1)
                        break;
                    
                    memset(buf2, 0, sizeof(buf2));
                    idxbuf2 = rsz - buflen;
                    memmove(buf2, &buf[buflen], idxbuf2);
                }
                else
                {
                    buflen = rsz;
                    if (1)//uh_save_upload_file(&fp, cl, &alreadyopen, buf, buflen, boundary, hexboundary, postdata, uploadFileName) == 1)
                        break;
                }
#else
                buflen = rsz;
                //gui_debug("buflen=%d", buflen);
                    
                memset(buf2, 0, sizeof(buf2));
                if (uh_save_upload_file(&fp, cl, &alreadyopen, buf, buflen, boundary, hexboundary, postdata) == 1)
                    break;
#endif                  
                
                if (sz == (headerlen + contentlen))
                {
                    break;
                }
            }
            else if( sz == 0 )
            {
                //gui_debug("sz=%d", sz);
                sz = -1;
                break;
            }
            else
            {
                //gui_debug("sz=%d", sz);
                break;
            }
        }
        
        if (fp)
            fclose(fp);
    }
    return sz;
}

int uh_save_upload_file(FILE **fp, struct client *cl, int *alreadyopen, char *buf, int buflen, char *boundary, unsigned char *hexboundary, char *postdata, char *uploadFileName)
{
    //gui_debug("buflen=%d", buflen);
    //gui_debug("postdata=%s", postdata);

#if 1
    int isdone = 0;
    char *p = NULL;
    char *q = NULL;
    char *r = NULL;
    
    if (*alreadyopen == 0)
    {
        p = gui_strstr(buf, "filename=");
        gui_debug("++++++++filname=%s\n",p);
        if (p)
        {
            char filename[BUFLEN_64] = {0};
            char pathname[BUFLEN_128] = {0};

#if 1           
            srand((int)time(0));                    
            int fileext = 1+(int)(1000000.0*rand()/(RAND_MAX+1.0));
            sprintf(filename, "%s%d", UPLOAD_FILE_PREFIX, fileext);
            sprintf(pathname, "/tmp/%s", filename);
            strcpy(uploadFileName,pathname);
#else           
            char *pfilename = NULL;
            pfilename = gui_strstr(p, "\r\n");
            memcpy(filename, &buf[(int)(p-buf)+strlen("filename=\"")], (int)(pfilename - p - strlen("filename=\"") - 1));
            sprintf(pathname, "/tmp/%s", filename);
            //gui_debug("filename=%s", filename);
#endif          
            
            char fieldname[BUFLEN_64] = {0};
            char buf0[UH_LIMIT_MSGHEAD] = {0};
            memcpy(buf0, &buf[0], (int)(p-buf));
            char *pfieldname1 = NULL;
            char *pfieldname2 = NULL;
            pfieldname1 = strrchr(buf0, '=');
            pfieldname2 = strrchr(buf0, '\"');
            memcpy(fieldname, &buf[(int)(pfieldname1-buf)+2], (int)(pfieldname2-pfieldname1)-2);
            gui_debug("++++++++++fieldname=%s", fieldname);
            
            memset(cl->peekbuf, 0, sizeof(cl->peekbuf));
            if (safe_strlen(postdata) > 0)
            {
                sprintf(cl->peekbuf, "%s=%s", fieldname, pathname);
                strcat(cl->peekbuf, "&");
                strcat(cl->peekbuf, postdata);
            }
            else
                sprintf(cl->peekbuf, "%s=%s", fieldname, pathname);
            
            *fp = fopen(pathname, "w+b");
            q = gui_strstr(p, "\r\n\r\n");

            if (q == NULL)
                return 1;

            q += 4;

            r = gui_strstr(q, boundary);
            if (r)
            {
                //gui_debug("r-q=%d", (int)(r-q));
                //gui_debug("fwrite len=%d", (int)(r-q));
                fwrite(q, sizeof(char), (int)(r-q), *fp);
                isdone = 1;
            }
            else
            {
                //gui_debug("strlen(q)=%d", strlen(q));
                //gui_debug("q=%s", q);
                //gui_debug("fwrite len=%d", buflen-(int)(q-buf));
                fwrite(q, sizeof(char), buflen-(int)(q-buf), *fp);
            }
            
            *alreadyopen = 1;
        }
    }
    else
    {
        unsigned char hexbuf[UH_LIMIT_MSGHEAD*2] = {0x00};
        gui_asc2hex(buf, buflen, hexbuf, buflen*2+1);
    
        //gui_debug("hexboundary=%s", hexboundary);
        //gui_debug("strlen(hexbuf)=%d, hexbuf=%s", strlen(hexbuf), hexbuf);
        r = gui_memstr((char *)hexbuf, strlen((char *)hexbuf), (char *)hexboundary);
        if (r)
        {
            hexbuf[(int)((unsigned char*)r-&hexbuf[0])] = '\0';
            //gui_debug("strlen(hexbuf)=%d, hexbuf=%s", strlen(hexbuf), hexbuf);
            
            char ascbuf[UH_LIMIT_MSGHEAD] = {0};
            gui_hex2asc(hexbuf, strlen((char *)hexbuf), ascbuf, strlen((char *)hexbuf)/2);

            fwrite(ascbuf, sizeof(char), strlen((char *)hexbuf)/2, *fp);
            
            isdone = 1;
        }
        else
        {
            fwrite(buf, sizeof(char), buflen, *fp);
        }
    }
    
    return isdone;
#endif  
}

#endif

int uh_tcp_peek(struct client *cl, char *buf, int len)
{
    int sz = uh_tcp_recv(cl, buf, len);

    /* store received data in peek buffer */
    if( sz > 0 )
    {
        cl->peeklen = sz;
        memcpy(cl->peekbuf, buf, sz);
    }

    return sz;
}

int uh_tcp_recv(struct client *cl, char *buf, int len)
{
    int sz = 0;
    int rsz = 0;

    fd_set reader;
    struct timeval timeout;

    /* first serve data from peek buffer */
    if( cl->peeklen > 0 )
    {   
        sz = min(cl->peeklen, len);
        len -= sz; cl->peeklen -= sz;

        memcpy(buf, cl->peekbuf, sz);
        memmove(cl->peekbuf, &cl->peekbuf[sz], cl->peeklen);
#ifdef AEI_REVJ     
        cl->peekbuf[(cl->peeklen>(UH_LIMIT_MSGHEAD-1))?(UH_LIMIT_MSGHEAD-1):cl->peeklen] = '\0';
#endif          
    }

    /* caller wants more */
    if( len > 0 )
    {
        FD_ZERO(&reader);
        FD_SET(cl->socket, &reader);

        //timeout.tv_sec  = cl->server->conf->network_timeout;
        timeout.tv_usec = 0;
#ifdef AEI_REVJ
        int header_length = 0;
        while(1)
        {
            if( select(cl->socket + 1, &reader, NULL, NULL, &timeout) > 0 )
            {
#ifdef HAVE_TLS
                if( cl->tls )
                    rsz = cl->server->conf->tls_recv(cl, (void *)&buf[sz], len);
                else
#endif
                    //rsz = recv(cl->socket, (void *)&buf[sz], len, 0);
	    	    rsz = read(0, (void *)&buf[sz], len);
                
                //gui_debug("sz=%d, rsz=%d", sz, rsz);
                if( (sz == 0) || (rsz > 0) )    sz += rsz;
                if (sz > (UH_LIMIT_MSGHEAD-1))
                {
                    buf[UH_LIMIT_MSGHEAD-1] = '\0';
                    break;
                }
                else if (header_length == sz)
                {
                    buf[sz] = '\0';
                    break;
                }
                else if (rsz <= 0)
                {
                    buf[sz] = '\0';
                    break;
                }
                
                buf[sz] = '\0';
            
                //gui_debug("sz=%d, buf=%s", sz, buf);
                char *idxptr;
                if( (idxptr = strfind(buf, sz, "\r\n\r\n", 4)) && header_length == 0)
                {
                    char *content_type;
                    content_type = get_http_header(buf, (int)((idxptr - buf) + 4), "Content-Type");
                    //gui_debug("content_type=%s", content_type);
                        
                    char *length;
                    length = get_http_header(buf, (int)((idxptr - buf) + 4), "Content-Length");
                    
                    //gui_debug("length=%s", length);
                    if (length)
                    {
                        header_length = atoi(length) + (int)((idxptr - buf) + 4);
                    }
                    else
                        break;
                    
                    //gui_debug("header_length=%d", header_length);
                    if (header_length == sz)
                    {
                        buf[sz] = '\0';
                        break;
                    }
                }
            }
            else if( sz == 0 )
            {
                //gui_debug("sz=%d", sz);
                sz = -1;
                break;
            }
            else
            {
                //gui_debug("sz=%d", sz);
                break;
            }
        }
#else
        if( select(cl->socket + 1, &reader, NULL, NULL, &timeout) > 0 )
        {
#ifdef HAVE_TLS
            if( cl->tls )
                rsz = cl->server->conf->tls_recv(cl, (void *)&buf[sz], len);
            else
#endif
                //rsz = recv(cl->socket, (void *)&buf[sz], len, 0);
	    	rsz = read(0, (void *)&buf[sz], len);
            if( (sz == 0) || (rsz > 0) )
                sz += rsz;
        }
        else if( sz == 0 )
        {
            sz = -1;
        }
#endif
    }

    return sz;
}


int uh_http_sendhf(struct client *cl, int code, const char *summary, const char *fmt, ...)
{
    va_list ap;

    char buffer[UH_LIMIT_MSGHEAD];
    int len;

    len = snprintf(buffer, sizeof(buffer),
        "HTTP/1.1 %03i %s\r\n"
        "Connection: close\r\n"
        "Content-Type: text/plain\r\n"
        "Transfer-Encoding: chunked\r\n\r\n",
            code, summary
    );
    printf("%s %d\n", buffer, len);
    gui_debug("\n");
    ensure_ret(uh_tcp_send(cl, buffer, len));
    gui_debug("\n");
    va_start(ap, fmt);
    len = vsnprintf(buffer, sizeof(buffer), fmt, ap);
    va_end(ap);

    ensure_ret(uh_http_sendc(cl, buffer, len));
    ensure_ret(uh_http_sendc(cl, NULL, 0));

    return 0;
}


int uh_http_sendc(struct client *cl, const char *data, int len)
{
    char chunk[8];
    int clen;

    if( len == -1 )
        len = strlen(data);

    if( len > 0 )
    {
        clen = snprintf(chunk, sizeof(chunk), "%X\r\n", len);
        ensure_ret(uh_tcp_send(cl, chunk, clen));
        ensure_ret(uh_tcp_send(cl, data, len));
        ensure_ret(uh_tcp_send(cl, "\r\n", 2));
    }
    else
    {
        ensure_ret(uh_tcp_send(cl, "0\r\n\r\n", 5));
    }

    return 0;
}

int uh_http_sendf(
    struct client *cl, struct http_request *req, const char *fmt, ...
) {
    va_list ap;
    char buffer[UH_LIMIT_MSGHEAD];
    int len;

    va_start(ap, fmt);
    len = vsnprintf(buffer, sizeof(buffer), fmt, ap);
    va_end(ap);

    if( (req != NULL) && (req->version > 1.0) )
        ensure_ret(uh_http_sendc(cl, buffer, len));
    else if( len > 0 )
        ensure_ret(uh_tcp_send(cl, buffer, len));

    return 0;
}

int uh_http_send(
    struct client *cl, struct http_request *req, const char *buf, int len
) {
    if( len < 0 )
        len = strlen(buf);

    if( (req != NULL) && (req->version > 1.0) )
        ensure_ret(uh_http_sendc(cl, buf, len));
    else if( len > 0 )
        ensure_ret(uh_tcp_send(cl, buf, len));

    return 0;
}


int uh_urldecode(char *buf, int blen, const char *src, int slen)
{
    int i;
    int len = 0;

#define hex(x) \
    (((x) <= '9') ? ((x) - '0') : \
        (((x) <= 'F') ? ((x) - 'A' + 10) : \
            ((x) - 'a' + 10)))

    for( i = 0; (i <= slen) && (i <= blen); i++ )
    {
        if( src[i] == '%' )
        {
            if( ((i+2) <= slen) && isxdigit(src[i+1]) && isxdigit(src[i+2]) )
            {
                buf[len++] = (char)(16 * hex(src[i+1]) + hex(src[i+2]));
                i += 2;
            }
            else
            {
                buf[len++] = '%';
            }
        }
#ifdef AEI_REVJ
        else if (src[i] == '+')
        {
            buf[len++] = ' ';
        }
#endif
        else
        {
            buf[len++] = src[i];
        }
    }

    return len;
}

int uh_urlencode(char *buf, int blen, const char *src, int slen)
{
    int i;
    int len = 0;
    const char hex[] = "0123456789abcdef";

    for( i = 0; (i <= slen) && (i <= blen); i++ )
    {
        if( isalnum(src[i]) || (src[i] == '-') || (src[i] == '_') ||
            (src[i] == '.') || (src[i] == '~') )
        {
            buf[len++] = src[i];
        }
        else if( (len+3) <= blen )
        {
            buf[len++] = '%';
            buf[len++] = hex[(src[i] >> 4) & 15];
            buf[len++] = hex[(src[i] & 15) & 15];
        }
        else
        {
            break;
        }
    }

    return len;
}

int uh_b64decode(char *buf, int blen, const unsigned char *src, int slen)
{
    int i = 0;
    int len = 0;

    unsigned int cin  = 0;
    unsigned int cout = 0;


    for( i = 0; (i <= slen) && (src[i] != 0); i++ )
    {
        cin = src[i];

        if( (cin >= '0') && (cin <= '9') )
            cin = cin - '0' + 52;
        else if( (cin >= 'A') && (cin <= 'Z') )
            cin = cin - 'A';
        else if( (cin >= 'a') && (cin <= 'z') )
            cin = cin - 'a' + 26;
        else if( cin == '+' )
            cin = 62;
        else if( cin == '/' )
            cin = 63;
        else if( cin == '=' )
            cin = 0;
        else
            continue;

        cout = (cout << 6) | cin;

        if( (i % 4) == 3 )
        {
            if( (len + 3) < blen )
            {
                buf[len++] = (char)(cout >> 16);
                buf[len++] = (char)(cout >> 8);
                buf[len++] = (char)(cout);
            }
            else
            {
                break;
            }
        }
    }

    buf[len++] = 0;
    return len;
}

static char * canonpath(const char *path, char *path_resolved)
{
    char path_copy[PATH_MAX];
    char *path_cpy = path_copy;
    char *path_res = path_resolved;

    struct stat s;


    /* relative -> absolute */
    if( *path != '/' )
    {
        getcwd(path_copy, PATH_MAX);
        strncat(path_copy, "/", PATH_MAX - strlen(path_copy));
        strncat(path_copy, path, PATH_MAX - strlen(path_copy));
    }
    else
    {
        strncpy(path_copy, path, PATH_MAX);
    }

    /* normalize */
    while( (*path_cpy != '\0') && (path_cpy < (path_copy + PATH_MAX - 2)) )
    {
        if( *path_cpy == '/' )
        {
            /* skip repeating / */
            if( path_cpy[1] == '/' )
            {
                path_cpy++;
                continue;
            }

            /* /./ or /../ */
            else if( path_cpy[1] == '.' )
            {
                /* skip /./ */
                if( (path_cpy[2] == '/') || (path_cpy[2] == '\0') )
                {
                    path_cpy += 2;
                    continue;
                }

                /* collapse /x/../ */
                else if( (path_cpy[2] == '.') &&
                         ((path_cpy[3] == '/') || (path_cpy[3] == '\0'))
                ) {
                    while( (path_res > path_resolved) && (*--path_res != '/') )
                        ;

                    path_cpy += 3;
                    continue;
                }
            }
        }

        *path_res++ = *path_cpy++;
    }

    /* remove trailing slash if not root / */
    if( (path_res > (path_resolved+1)) && (path_res[-1] == '/') )
        path_res--;
    else if( path_res == path_resolved )
        *path_res++ = '/';

    *path_res = '\0';

    /* test access */
    if( !stat(path_resolved, &s) && (s.st_mode & S_IROTH) )
        return path_resolved;

    return NULL;
}

struct path_info * uh_path_lookup(struct client *cl, const char *url)
{
    static char path_phys[PATH_MAX];
    static char path_info[PATH_MAX];
    static struct path_info p;

    char buffer[UH_LIMIT_MSGHEAD];
    char *docroot = "/tmp";//cl->server->conf->docroot;
    char *pathptr = NULL;

    int slash = 0;
    int no_sym = 1;//cl->server->conf->no_symlinks;
    int i = 0;
    struct stat s;

    /* back out early if url is undefined */
    if ( url == NULL )
        return NULL;

    memset(path_phys, 0, sizeof(path_phys));
    memset(path_info, 0, sizeof(path_info));
    memset(buffer, 0, sizeof(buffer));
    memset(&p, 0, sizeof(p));

    /* copy docroot */
    memcpy(buffer, docroot,
        min(strlen(docroot), sizeof(buffer) - 1));

    /* separate query string from url */
    if( (pathptr = strchr(url, '?')) != NULL )
    {
        p.query = pathptr[1] ? pathptr + 1 : NULL;

        /* urldecode component w/o query */
        if( pathptr > url )
            uh_urldecode(
                &buffer[strlen(docroot)],
                sizeof(buffer) - strlen(docroot) - 1,
                url, (int)(pathptr - url) - 1
            );
    }

    /* no query string, decode all of url */
    else
    {
        uh_urldecode(
            &buffer[strlen(docroot)],
            sizeof(buffer) - strlen(docroot) - 1,
            url, strlen(url)
        );
    }

    /* create canon path */
    for( i = strlen(buffer), slash = (buffer[max(0, i-1)] == '/'); i >= 0; i-- )
    {
        if( (buffer[i] == 0) || (buffer[i] == '/') )
        {
            memset(path_info, 0, sizeof(path_info));
            memcpy(path_info, buffer, min(i + 1, sizeof(path_info) - 1));

            if( no_sym ? realpath(path_info, path_phys)
                       : canonpath(path_info, path_phys)
            ) {
                memset(path_info, 0, sizeof(path_info));
                memcpy(path_info, &buffer[i],
                    min(strlen(buffer) - i, sizeof(path_info) - 1));

                break;
            }
        }
    }

    /* check whether found path is within docroot */
    if( strncmp(path_phys, docroot, strlen(docroot)) ||
        ((path_phys[strlen(docroot)] != 0) &&
         (path_phys[strlen(docroot)] != '/'))
    ) {
        return NULL;
    }

    /* test current path */
    if( ! stat(path_phys, &p.stat) )
    {
        /* is a regular file */
        if( p.stat.st_mode & S_IFREG )
        {
            p.root = docroot;
            p.phys = path_phys;
            p.name = &path_phys[strlen(docroot)];
            p.info = path_info[0] ? path_info : NULL;
        }

        /* is a directory */
        else if( (p.stat.st_mode & S_IFDIR) && !strlen(path_info) )
        {
            /* ensure trailing slash */
            if( path_phys[strlen(path_phys)-1] != '/' )
                path_phys[strlen(path_phys)] = '/';

            /* try to locate login file */
            memset(buffer, 0, sizeof(buffer));
            memcpy(buffer, path_phys, sizeof(path_phys));
            pathptr = &buffer[strlen(buffer)];

            /* if requested url resolves to a directory and a trailing slash
               is missing in the request url, redirect the client to the same
               url with trailing slash appended */
            if( !slash )
            {
                uh_http_sendf(cl, NULL,
                    "HTTP/1.1 302 Found\r\n"
                    "Location: %s%s%s\r\n"
                    "Connection: close\r\n\r\n",
                        &path_phys[strlen(docroot)],
                        p.query ? "?" : "",
                        p.query ? p.query : ""
                );

                p.redirected = 1;
            }
            else if( 0)//cl;->server->conf->index_file )
            {
                strncat(buffer, cl->server->conf->index_file, sizeof(buffer));

                if( !stat(buffer, &s) && (s.st_mode & S_IFREG) )
                {
                    memcpy(path_phys, buffer, sizeof(path_phys));
                    memcpy(&p.stat, &s, sizeof(p.stat));
                }
            }
            else
            {
                for( i = 0; i < array_size(uh_index_files); i++ )
                {
                    strncat(buffer, uh_index_files[i], sizeof(buffer));

                    if( !stat(buffer, &s) && (s.st_mode & S_IFREG) )
                    {
                        memcpy(path_phys, buffer, sizeof(path_phys));
                        memcpy(&p.stat, &s, sizeof(p.stat));
                        break;
                    }

                    *pathptr = 0;
                }
            }

            p.root = docroot;
            p.phys = path_phys;
            p.name = &path_phys[strlen(docroot)];
        }
    }

    return p.phys ? &p : NULL;
}


static struct auth_realm *uh_realms = NULL;

struct auth_realm * uh_auth_add(char *path, char *user, char *pass)
{
    struct auth_realm *new = NULL;
    struct passwd *pwd;

#ifdef HAVE_SHADOW
    struct spwd *spwd;
#endif

    if((new = (struct auth_realm *)malloc(sizeof(struct auth_realm))) != NULL)
    {
        memset(new, 0, sizeof(struct auth_realm));

        memcpy(new->path, path,
            min(strlen(path), sizeof(new->path) - 1));

        memcpy(new->user, user,
            min(strlen(user), sizeof(new->user) - 1));

        /* given password refers to a passwd entry */
        if( (strlen(pass) > 3) && !strncmp(pass, "$p$", 3) )
        {
#ifdef HAVE_SHADOW
            /* try to resolve shadow entry */
            if( ((spwd = getspnam(&pass[3])) != NULL) && spwd->sp_pwdp )
            {
                memcpy(new->pass, spwd->sp_pwdp,
                    min(strlen(spwd->sp_pwdp), sizeof(new->pass) - 1));
            }

            else
#endif

            /* try to resolve passwd entry */
            if( ((pwd = getpwnam(&pass[3])) != NULL) && pwd->pw_passwd &&
                (pwd->pw_passwd[0] != '!') && (pwd->pw_passwd[0] != 0)
            ) {
                memcpy(new->pass, pwd->pw_passwd,
                    min(strlen(pwd->pw_passwd), sizeof(new->pass) - 1));
            }
        }

        /* ordinary pwd */
        else
        {
            memcpy(new->pass, pass,
                min(strlen(pass), sizeof(new->pass) - 1));
        }

        if( new->pass[0] )
        {
            new->next = uh_realms;
            uh_realms = new;

            return new;
        }

        free(new);
    }

    return NULL;
}

int uh_auth_check(
    struct client *cl, struct http_request *req, struct path_info *pi
) {
    int i, plen, rlen, protected;
    char buffer[UH_LIMIT_MSGHEAD];
    char *user = NULL;
    char *pass = NULL;

    struct auth_realm *realm = NULL;

    plen = strlen(pi->name);
    protected = 0;

    /* check whether at least one realm covers the requested url */
    for( realm = uh_realms; realm; realm = realm->next )
    {
        rlen = strlen(realm->path);

        if( (plen >= rlen) && !strncasecmp(pi->name, realm->path, rlen) )
        {
            req->realm = realm;
            protected = 1;
            break;
        }
    }

    /* requested resource is covered by a realm */
    if( protected )
    {
	    printf("zzzzzzzzzz\n");
        /* try to get client auth info */
        foreach_header(i, req->headers)
        {
            if( !strcasecmp(req->headers[i], "Authorization") &&
                (strlen(req->headers[i+1]) > 6) &&
                !strncasecmp(req->headers[i+1], "Basic ", 6)
            ) {
                memset(buffer, 0, sizeof(buffer));
                uh_b64decode(buffer, sizeof(buffer) - 1,
                    (unsigned char *) &req->headers[i+1][6],
                    strlen(req->headers[i+1]) - 6);

                if( (pass = strchr(buffer, ':')) != NULL )
                {
                    user = buffer;
                    *pass++ = 0;
                }

                break;
            }
        }

        /* have client auth */
        if( user && pass )
        {
            /* find matching realm */
            for( realm = uh_realms; realm; realm = realm->next )
            {
                rlen = strlen(realm->path);

                if( (plen >= rlen) &&
                    !strncasecmp(pi->name, realm->path, rlen) &&
                    !strcmp(user, realm->user)
                ) {
                    req->realm = realm;
                    break;
                }
            }

            /* found a realm matching the username */
            if( realm )
            {
                /* is a crypt passwd */
                if( realm->pass[0] == '$' )
                    //pass = crypt(pass, realm->pass);
		    pass = "asdf";

                /* check user pass */
                if( !strcmp(pass, realm->pass) )
                    return 1;
            }
        }

        /* 401 */
        uh_http_sendf(cl, NULL,
            "HTTP/%.1f 401 Authorization Required\r\n"
            "WWW-Authenticate: Basic realm=\"%s\"\r\n"
            "Content-Type: text/plain\r\n"
            "Content-Length: 23\r\n\r\n"
            "Authorization Required\n",
                req->version, 0//cl->server->conf->realm
        );

        return 0;
    }

    return 1;
}


static struct listener *uh_listeners = NULL;
static struct client *uh_clients = NULL;

struct listener * uh_listener_add(int sock, struct config *conf)
{
    struct listener *new = NULL;
    socklen_t sl;

    if( (new = (struct listener *)malloc(sizeof(struct listener))) != NULL )
    {
        memset(new, 0, sizeof(struct listener));

        new->socket = sock;
        new->conf   = conf;

        /* get local endpoint addr */
        sl = sizeof(struct sockaddr_in6);
        memset(&(new->addr), 0, sl);
        getsockname(sock, (struct sockaddr *) &(new->addr), &sl);

        new->next = uh_listeners;
        uh_listeners = new;

        return new;
    }

    return NULL;
}

struct listener * uh_listener_lookup(int sock)
{
    struct listener *cur = NULL;

    for( cur = uh_listeners; cur; cur = cur->next )
        if( cur->socket == sock )
            return cur;

    return NULL;
}


struct client * uh_client_add(int sock, struct listener *serv)
{
    struct client *new = NULL;
    socklen_t sl;

    if( (new = (struct client *)malloc(sizeof(struct client))) != NULL )
    {
        memset(new, 0, sizeof(struct client));

        //new->socket = sock;
        new->server = serv;

        /* get remote endpoint addr */
        //sl = sizeof(struct sockaddr_in6);
        //memset(&(new->peeraddr), 0, sl);
        //getpeername(sock, (struct sockaddr *) &(new->peeraddr), &sl);

        /* get local endpoint addr */
        //sl = sizeof(struct sockaddr_in6);
        //memset(&(new->servaddr), 0, sl);
        //getsockname(sock, (struct sockaddr *) &(new->servaddr), &sl);

        new->next = uh_clients;
        uh_clients = new;
    }

    return new;
}

struct client * uh_client_lookup(int sock)
{
    struct client *cur = NULL;

    for( cur = uh_clients; cur; cur = cur->next )
        if( cur->socket == sock )
            return cur;

    return NULL;
}

void uh_client_remove(int sock)
{
    struct client *cur = NULL;
    struct client *prv = NULL;

    for( cur = uh_clients; cur; prv = cur, cur = cur->next )
    {
        if( cur->socket == sock )
        {
            if( prv )
                prv->next = cur->next;
            else
                uh_clients = cur->next;

            free(cur);
            break;
        }
    }
}


#ifdef HAVE_CGI
static struct interpreter *uh_interpreters = NULL;

struct interpreter * uh_interpreter_add(const char *extn, const char *path)
{
    struct interpreter *new = NULL;

    if( (new = (struct interpreter *)
            malloc(sizeof(struct interpreter))) != NULL )
    {
        memset(new, 0, sizeof(struct interpreter));

        memcpy(new->extn, extn, min(strlen(extn), sizeof(new->extn)-1));
        memcpy(new->path, path, min(strlen(path), sizeof(new->path)-1));

        new->next = uh_interpreters;
        uh_interpreters = new;

        return new;
    }

    return NULL;
}

struct interpreter * uh_interpreter_lookup(const char *path)
{
    struct interpreter *cur = NULL;
    const char *e;

    for( cur = uh_interpreters; cur; cur = cur->next )
    {
        e = &path[max(strlen(path) - strlen(cur->extn), 0)];

        if( !strcmp(e, cur->extn) )
            return cur;
    }

    return NULL;
}
#endif

#if HAVE_SESSION
unsigned int getSessionIDFromReq(struct http_request *req)
{
    char *session_id = NULL;
    int i;
    foreach_header(i, req->headers)
    {
        //gui_debug("%d: %s=%s", i, req->headers[i], req->headers[i+1]);
        //if (safe_strcmp(req->headers[i], "Cookie") == 0)
        if (req->headers[i] && strcasecmp(req->headers[i], "cookie") == 0)
        {
            char header[UH_LIMIT_MSGHEAD] = {0};
            sprintf(header, req->headers[i+1], strlen(req->headers[i+1]));
            //gui_debug("%s = %s", req->headers[i], header);
            
            char *dest[128];
            int num = 0;
            int j;
            
            gui_split(header, ";", dest, &num);
            //gui_debug("num=%d", num);
            for (j=0; j<num; ++j)
            {
                gui_debug("UH_SESSION_COOKIE=%s, dest[j]=%s", UH_SESSION_COOKIE, dest[j]);
                if (strstr(dest[j], UH_SESSION_COOKIE) != NULL)
                {
                    char *dest2[32];
                    int num2 = 0;
                    
                    gui_split(dest[j], "=", dest2, &num2);
                    gui_trim(dest2[0]);
                    gui_debug("num2=%d", num2);
                    gui_debug("dest2[0]=%s, dest2[1]=%s", dest2[0], dest2[1]);
                    //if (num2 != 2)  continue;
                    //if (strcasecmp(dest2[0], UH_SESSION_COOKIE) != 0)    continue;
                    if (num2 != 2){
                        gui_debug("num2 != 2");
                        continue;
                    }

                    int xxx = strcasecmp(dest2[0], UH_SESSION_COOKIE);
                    gui_debug("cookiename compare = %d", xxx);
                    if (xxx != 0){
                        gui_debug("cookiename compare != 0");
                        continue;
                    }

                    
                    session_id = dest2[1];
                    gui_debug("session_id=%s", session_id);
                    break;
                }
            }

            break;
        }
    }
    
    gui_debug("session_id=%s", session_id);
    unsigned int result = (session_id)?(unsigned int)atoi(session_id):1;
    gui_debug("result=%u", result);
    return result;
}
#endif

#ifdef AEI_UHTTPD_REALM_AUTH
char *getAccessRealm(char *protol, int port)
{
    int i;
    char *protocol = protol;
    if (protocol)
    {
        for (i = 0; i < UHTTPD_REALM_COUNT; i ++)
        {
            if(!strcmp(protocol, uhttp_realm[i].protol) && (port >= uhttp_realm[i].port_range.low && port <= uhttp_realm[i].port_range.high))
                return uhttp_realm[i].realm;
        }
    }
    return NULL;
}
#endif
