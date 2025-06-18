/*
 * uhttpd - Tiny single-threaded httpd - Main header
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

#ifndef _UHTTPD_
#define _UHTTPD_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <sys/wait.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <linux/limits.h>
#include <netdb.h>
#include <ctype.h>
#include <errno.h>
#include <dlfcn.h>

#define AEI_REVJ 1
#define BUFLEN_1024 1024
#define BUFLEN_64 64
#define BUFLEN_256 256
#define BUFLEN_128 128
#define UPLOAD_FILE_PREFIX "/tmp/"

#ifdef HAVE_LUA
#include <lua.h>
#endif

#ifdef HAVE_TLS
#include <openssl/ssl.h>
#endif

/* uClibc... */
#ifndef SOL_TCP
#define SOL_TCP 6
#endif

#define gui_debug(arg...)    do { \
            fprintf(stderr,"--GUI %s(%d):%s: ", __FILE__, __LINE__, __FUNCTION__);\
            fprintf(stderr,##arg);\
            fprintf(stderr,"\n" );\
        } while(0)

#define gui_warning(arg...)    do { \
            fprintf(stderr,"--GUI %s(%d):%s: ", __FILE__, __LINE__, __FUNCTION__);\
            fprintf(stderr,##arg);\
            fprintf(stderr,"\n" );\
        } while(0)

#define UH_LIMIT_MSGHEAD    131072
#define UH_LIMIT_HEADERS    64

#define UH_LIMIT_CLIENTS    64

#define UH_HTTP_MSG_GET     0
#define UH_HTTP_MSG_HEAD    1
#define UH_HTTP_MSG_POST    2

#ifdef HAVE_SESSION
#define UH_LOGIN_PAGE       "/home.html"
#define UH_MAIN_PAGE        "/main.html"
#define UH_SESSION_COOKIE   "aei_session_id_cookie"
#define UH_SESSION_CHECKTIME    60

unsigned int g_logined_session_id;
#endif

#ifdef AEI_REVJ
#define UH_SYSTEM_SETTING_FILE  "/aeimnt/rt_conf/uhttpd.setting"
#define UH_SESSION_LIFETIME     900
#define UH_IPV4_HTTP_PORT1      80
#define UH_IPV4_HTTP_PORT2      8080
#define UH_IPV4_HTTPS_PORT1     443
#define UH_IPV4_HTTPS_PORT2     8443
#define UH_LOGIN_ATTEMPTS       5
#define UH_SESSION_MAX          15
#define UH_CURRENT_USER         "admin"

#define UH_SESSION_LIFETIME_STR     "session_lifetime"
#define UH_IPV4_HTTP_PORT1_STR      "ipv4_http_port1"
#define UH_IPV4_HTTP_PORT2_STR      "ipv4_http_port2"
#define UH_IPV4_HTTPS_PORT1_STR     "ipv4_https_port1"
#define UH_IPV4_HTTPS_PORT2_STR     "ipv4_https_port2"
#define UH_LOGIN_ATTEMPTS_STR       "login_attempts"
#define UH_SESSION_MAX_STR          "session_max"
#define UH_CURRENT_USER_STR         "current_user"

#define UH_CONF_BUFLEN   8

#define UH_FIRSTTIME_LOGIN_PAGE    "/firsttime_login.html"
#endif

#ifdef AEI_OPENWRT
#define UH_BUFLEN_4    4
#define UH_BUFLEN_8    8
#define UH_BUFLEN_16   16
#define UH_BUFLEN_32   32
#define UH_BUFLEN_64   64
#define UH_BUFLEN_128  128
#define UH_BUFLEN_256  256
#endif

struct listener;
struct client;
struct interpreter;
struct http_request;

struct config {
    char docroot[PATH_MAX];
    char *realm;
    char *file;
    char *index_file;
    char *error_handler;
    int no_symlinks;
    int no_dirlists;
    int network_timeout;
    int rfc1918_filter;
    int tcp_keepalive;
#ifdef HAVE_CGI
    char *cgi_prefix;
#endif
#ifdef HAVE_LUA
    char *lua_prefix;
    char *lua_handler;
    lua_State *lua_state;
    lua_State * (*lua_init) (const char *handler);
    void (*lua_close) (lua_State *L);
    void (*lua_request) (struct client *cl, struct http_request *req, lua_State *L);
#endif
#if defined(HAVE_CGI) || defined(HAVE_LUA)
    int script_timeout;
#endif
#ifdef HAVE_TLS
    char *cert;
    char *key;
    SSL_CTX *tls;
    SSL_CTX * (*tls_init) (void);
    int (*tls_cert) (SSL_CTX *c, const char *file);
    int (*tls_key) (SSL_CTX *c, const char *file);
    void (*tls_free) (struct listener *l);
    void (*tls_accept) (struct client *c);
    void (*tls_close) (struct client *c);
    int (*tls_recv) (struct client *c, void *buf, int len);
    int (*tls_send) (struct client *c, void *buf, int len);
#endif
};

struct listener {
    int socket;
    struct sockaddr_in6 addr;
    struct config *conf;
#ifdef HAVE_TLS
    SSL_CTX *tls;
#endif
    struct listener *next;
};

struct client {
    int socket;
    int peeklen;
    char peekbuf[UH_LIMIT_MSGHEAD];
    struct listener *server;
    struct sockaddr_in6 servaddr;
    struct sockaddr_in6 peeraddr;
#ifdef HAVE_TLS
    SSL *tls;
#endif
    struct client *next;
};

struct auth_realm {
    char path[PATH_MAX];
    char user[32];
    char pass[128];
    struct auth_realm *next;
};

struct http_request {
    int method;
    float version;
    int redirect_status;
    char *url;
    char *headers[UH_LIMIT_HEADERS];
    struct auth_realm *realm;
#if HAVE_SESSION
    unsigned int session_id;
#endif  
};

struct http_response {
    int statuscode;
    char *statusmsg;
    char *headers[UH_LIMIT_HEADERS];
};

#ifdef HAVE_CGI
struct interpreter {
    char path[PATH_MAX];
    char extn[32];
    struct interpreter *next;
};
#endif

#ifdef AEI_REVJ
struct system_setting {
    int session_lifetime;
    
    int ipv4_http_port1;
    int ipv4_http_port2;
    int ipv4_https_port1;
    int ipv4_https_port2;
    
    int login_attempts;
    int session_max;

    char current_user[32];
};
extern struct system_setting g_sys_setting;

struct system_setting init_systemsetting();
void modify_system_setting(char *fieldname, int old_value, int new_value);
void modify_system_setting_string(char *fieldname, char *new_value);
#endif

#ifdef AEI_UHTTPD_REALM_AUTH
typedef struct {
    char *protol;
    char *url_str;
    struct {
        unsigned int low;
        unsigned int high;
    }port_range;
    char *realm;
}URLMap2Realm;

typedef struct {
    int *https_port;
    int count;
}HTTPSPortRec;
#endif
#endif

