/*
 * uhttpd - Tiny single-threaded httpd - Main component
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

#include "uhttpd.h"
#include "uhttpd-utils.h"
#include "uhttpd-file.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include "cgi_utils.h"

#ifdef HAVE_CGI
#include "uhttpd-cgi.h"
#endif

#ifdef HAVE_LUA
#include "uhttpd-lua.h"
#endif

#ifdef HAVE_TLS
#include "uhttpd-tls.h"
#endif

#ifdef AEI_REVJ
struct system_setting g_sys_setting;
#endif

#ifdef HAVE_SESSION
#include "cgi_session.h"
static int session_waiting_time = 0;
extern int login_max_try;
extern int login_left_try;
#endif

#ifdef AEI_BCM_FIRMWARE_UPLOAD
#include "cgi_upload.h"
#endif

#ifdef AEI_UHTTPD_REALM_AUTH
#include <malloc.h>
HTTPSPortRec https_rec = {
    .https_port = NULL,  // save ports for https
    .count = 0   //record the count of ports
};
#endif
static int run = 1;
int CTL_LOG_LEVEL = 1; //sharko add for compile

static void uh_sigterm(int sig)
{
    run = 0;
#ifdef AEI_UHTTPD_REALM_AUTH
    if(https_rec.https_port != NULL)
        free(https_rec.https_port);
#endif
}

static void uh_sigchld(int sig)
{
    while( waitpid(-1, NULL, WNOHANG) > 0 ) { }
}

static void uh_config_parse(struct config *conf)
{
    FILE *c;
    char line[512];
    char *col1 = NULL;
    char *col2 = NULL;
    char *eol  = NULL;

    const char *path = conf->file ? conf->file : "/etc/httpd.conf";


    if( (c = fopen(path, "r")) != NULL )
    {
        memset(line, 0, sizeof(line));

        while( fgets(line, sizeof(line) - 1, c) )
        {
            if( (line[0] == '/') && (strchr(line, ':') != NULL) )
            {
                if( !(col1 = strchr(line, ':')) || (*col1++ = 0) ||
                    !(col2 = strchr(col1, ':')) || (*col2++ = 0) ||
                    !(eol = strchr(col2, '\n')) || (*eol++  = 0) )
                        continue;

                if( !uh_auth_add(line, col1, col2) )
                {
                    fprintf(stderr,
                        "Notice: No password set for user %s, ignoring "
                        "authentication on %s\n", col1, line
                    );
                }
            }
            else if( !strncmp(line, "I:", 2) )
            {
                if( !(col1 = strchr(line, ':')) || (*col1++ = 0) ||
                    !(eol = strchr(col1, '\n')) || (*eol++  = 0) )
                        continue;

                conf->index_file = strdup(col1);
            }
            else if( !strncmp(line, "E404:", 5) )
            {
                if( !(col1 = strchr(line, ':')) || (*col1++ = 0) ||
                    !(eol = strchr(col1, '\n')) || (*eol++  = 0) )
                        continue;

                conf->error_handler = strdup(col1);
            }
#ifdef HAVE_CGI
            else if( (line[0] == '*') && (strchr(line, ':') != NULL) )
            {
                if( !(col1 = strchr(line, '*')) || (*col1++ = 0) ||
                    !(col2 = strchr(col1, ':')) || (*col2++ = 0) ||
                    !(eol = strchr(col2, '\n')) || (*eol++  = 0) )
                        continue;

                if( !uh_interpreter_add(col1, col2) )
                {
                    fprintf(stderr,
                        "Unable to add interpreter %s for extension %s: "
                        "Out of memory\n", col2, col1
                    );
                }
            }
#endif
        }

        fclose(c);
    }
}

static int uh_socket_bind(
    fd_set *serv_fds, int *max_fd, const char *host, const char *port,
    struct addrinfo *hints, int do_tls, struct config *conf
) {
}

static struct http_request * uh_http_header_parse(struct client *cl, char *buffer, int buflen)
{
    char *method  = &buffer[0];
    char *path    = NULL;
    char *version = NULL;

    char *headers = NULL;
    char *hdrname = NULL;
    char *hdrdata = NULL;

    int i;
    int hdrcount = 0;

    static struct http_request req;

    memset(&req, 0, sizeof(req));

    /* terminate initial header line */
    if( (headers = strfind(buffer, buflen, "\r\n", 2)) != NULL )
    {
        buffer[buflen-1] = 0;

        *headers++ = 0;
        *headers++ = 0;
        //gui_debug("\n@@headers=%s\n\n",headers);

        /* find request path */
        if( (path = strchr(buffer, ' ')) != NULL )
            *path++ = 0;
        //gui_debug("\n@@path=%s\n\n",path);
        /* find http version */
        if( (path != NULL) && ((version = strchr(path, ' ')) != NULL) )
            *version++ = 0;
        //gui_debug("\n@@version=%s\n\n",version);
        /* check method */
        if( strcmp(method, "GET") && strcmp(method, "HEAD") && strcmp(method, "POST") )
        {
            /* invalid method */
            uh_http_response(cl, 405, "Method Not Allowed");
            return NULL;
        }
        else
        {
            switch(method[0])
            {
                case 'G':
                    req.method = UH_HTTP_MSG_GET;
                    break;

                case 'H':
                    req.method = UH_HTTP_MSG_HEAD;
                    break;

                case 'P':
                    req.method = UH_HTTP_MSG_POST;
                    break;
            }
        }

        /* check path */
        if( !path || !strlen(path) )
        {
            /* malformed request */
            uh_http_response(cl, 400, "Bad Request");
            return NULL;
        }
        else
        {
            req.url = path;
        }

        /* check version */
        if( (version == NULL) || (strcmp(version, "HTTP/0.9") &&
            strcmp(version, "HTTP/1.0") && strcmp(version, "HTTP/1.1")) )
        {
            /* unsupported version */
            uh_http_response(cl, 400, "Bad Request");
            return NULL;
        }
        else
        {
            req.version = strtof(&version[5], NULL);
        }


        /* process header fields */
        for( i = (int)(headers - buffer); i < buflen; i++ )
        {
            /* found eol and have name + value, push out header tuple */
            if( hdrname && hdrdata && (buffer[i] == '\r' || buffer[i] == '\n') )
            {
                buffer[i] = 0;

                /* store */
                if( (hdrcount + 1) < array_size(req.headers) )
                {
                    req.headers[hdrcount++] = hdrname;
                    req.headers[hdrcount++] = hdrdata;

                    hdrname = hdrdata = NULL;
                }

                /* too large */
                else
                {
                    uh_http_response(cl, 413, "Request Entity Too Large");
                    return NULL;
                }
            }

            /* have name but no value and found a colon, start of value */
            else if( hdrname && !hdrdata && ((i+2) < buflen) &&
                (buffer[i] == ':') && (buffer[i+1] == ' ')
            ) {
                buffer[i] = 0;
                hdrdata = &buffer[i+2];
            }

            /* have no name and found [A-Za-z], start of name */
            else if( !hdrname && isalpha(buffer[i]) )
            {
                hdrname = &buffer[i];
            }
        }

        /* valid enough */
        req.redirect_status = 200;
        return &req;
    }

    /* Malformed request */
    uh_http_response(cl, 400, "Bad Request");
    return NULL;
}


static struct http_request * uh_http_header_recv(struct client *cl)
{
    static char buffer[UH_LIMIT_MSGHEAD];
    char *bufptr = &buffer[0];
    char *idxptr = NULL;

    struct timeval timeout;

    fd_set reader;

    ssize_t blen = sizeof(buffer)-1;
    ssize_t rlen = 0;

    memset(buffer, 0, sizeof(buffer));

    //while( blen > 0 )
    {
        FD_ZERO(&reader);
        FD_SET(cl->socket, &reader);

        /* fail after 0.1s */
        timeout.tv_sec  = 0;
        timeout.tv_usec = 100000;

        /* check whether fd is readable */
        if( 1)//select(cl->socket + 1, &reader, NULL, NULL, &timeout) > 0 )
        {       
            ensure_out(rlen = uh_tcp_recv_header(cl, bufptr, blen));
            blen -= rlen;
            
            if( (idxptr = strfind(buffer, sizeof(buffer), "\r\n\r\n", 4)) )
            {
                int contentlen = 0;
                int headerlen = (int)(idxptr - bufptr) + 4;
                //gui_debug("headerlen=%d, buffer=%s", headerlen, buffer);
                
                struct http_request *req;
                req = uh_http_header_parse(cl, buffer, headerlen);

                if(req == NULL)
                    return NULL;

                int i;
                foreach_header(i, req->headers)
                {               
                //gui_debug("\n\n@@i=%d,req->headers[i]=%s\n\n",i,req->headers[i+1]);
                    if(strcasecmp(req->headers[i], "Content-Length") == 0)
                    {
                        contentlen = atoi(req->headers[i+1]);
                        break;
                    }
                }

                foreach_header(i, req->headers)
                {               
                    //gui_debug("headers name=%s, headers value=%s", req->headers[i], req->headers[i+1]);
                    if(strcasecmp(req->headers[i], "Content-Type") == 0)
                    {
                        //gui_debug("Content-Type=%s", req->headers[i+1]);
                        if (gui_strstr(req->headers[i+1], "multipart/form-data") != NULL)
                        {
                            //upload file
                            char *boundary_key = "boundary=";
                            char boundary[BUFLEN_128] = {0};
                            //char p_boundary = &boundary[0];
                            
                            char *p = NULL;
                            p = gui_strstr(req->headers[i+1], boundary_key);
                            p += strlen(boundary_key);
			    printf("%s\n", p);
                            sprintf(boundary, "\r\n--%s", p);
                            
                            char uploadFileName[BUFLEN_64] = {0};
                            //gWebParams.upgrade_status = -9;
                            ensure_out(rlen = uh_tcp_recv_file(cl, headerlen, contentlen, boundary, uploadFileName));
                            gui_debug("===uploadFileName=%s\n",uploadFileName);
                            //gWebParams.upgrade_status = 9;
                            //if upgrade image then kill some process
                        }
                        else if (gui_strstr(req->headers[i+1], "application/x-www-form-urlencoded") != NULL)
                        {
                            //normal post data
                            ensure_out(rlen = uh_tcp_recv_content(cl, bufptr, headerlen, contentlen));
                            //gui_debug("cl->peekbuf=%s", cl->peekbuf);
                        }

                        break;
                    }
                }
                
                return req;
            }

        }
        else
        {
            /* invalid request (unexpected eof/timeout) */
            return NULL;
        }
    }

    /* request entity too large */
    uh_http_response(cl, 413, "Request Entity Too Large");

out:
    return NULL;
}

#if defined(HAVE_LUA) || defined(HAVE_CGI)
static int uh_path_match(const char *prefix, const char *url)
{
    if( (strstr(url, prefix) == url) &&
        ((prefix[strlen(prefix)-1] == '/') ||
         (strlen(url) == strlen(prefix))   ||
         (url[strlen(prefix)] == '/'))
    ) {
        return 1;
    }

    return 0;
}
#endif

static void uh_dispatch_request(
    struct client *cl, struct http_request *req, struct path_info *pin
) {
        //uh_file_request(cl, req, pin);
}

static void uh_mainloop(struct config *conf, fd_set serv_fds, int max_fd)
{
    /* master file descriptor list */
    fd_set used_fds, read_fds;

    /* working structs */
    struct http_request *req;
    struct path_info *pin;
    struct client *cl;

    /* maximum file descriptor number */
    int new_fd, cur_fd = 0;

    /* clear the master and temp sets */
    FD_ZERO(&used_fds);
    FD_ZERO(&read_fds);

    /* backup server descriptor set */
    used_fds = serv_fds;
#ifdef AEI_UHTTPD_REALM_AUTH
    struct sockaddr_in serv_addr;
    int serv_len = sizeof(serv_addr);
    int serv_port = 0;
    int ret = -1;
    char *protocol = NULL;
    char *access_realm = NULL;
#endif
    /* loop */
    //while(run)
    //{
        /* create a working copy of the used fd set */
        read_fds = used_fds;

#ifdef HAVE_SESSION

        struct timeval timeout;
        int select_num = 0;
        
        timeout.tv_sec  = UH_SESSION_CHECKTIME;
        timeout.tv_usec = 0;
        
	/*
        select_num = select(max_fd + 1, &read_fds, NULL, NULL, &timeout);
        if(select_num == -1 )
        {
            perror("select()");
            exit(1);
        }
        else if(select_num == 0 )
        {
            //timeout               
            session_timeout_check();
            session_waiting_time = 0;
            continue;
        }
        else
        {
            session_waiting_time += (UH_SESSION_CHECKTIME - (int)timeout.tv_sec - 1);
            if (session_waiting_time >= UH_SESSION_CHECKTIME)
            {
                session_timeout_check();
                session_waiting_time = 0;
            }
        }
	*/
        
        //gui_debug("time=%d, select_num=%d, tv_sec=%d, tv_usec=%d, session_waiting_time=%d", time(0), select_num, timeout.tv_sec, timeout.tv_usec, session_waiting_time);

#else
        /* sleep until socket activity */
        //if( select(max_fd + 1, &read_fds, NULL, NULL, NULL) == -1 )
        //{
        //    perror("select()");
        //    exit(1);
        //}
#endif              

        /* run through the existing connections looking for data to be read */
        for( cur_fd = 0; cur_fd <= max_fd; cur_fd++ )
        {
            /* is a socket managed by us */
            if(1)// FD_ISSET(cur_fd, &read_fds) )
            {
                /* is one of our listen sockets */
                if(0)// FD_ISSET(cur_fd, &serv_fds) )
                {
                    /* handle new connections */
                    if( 1)//(new_fd = accept(cur_fd, NULL, 0)) != -1 )
                    {
                        /* add to global client list */
                        if( (cl = uh_client_add(new_fd, uh_listener_lookup(cur_fd))) != NULL )
                        {
#ifdef HAVE_TLS
                            /* setup client tls context */
                            if( conf->tls )
                                conf->tls_accept(cl);
#endif

                            /* add client socket to global fdset */
                            FD_SET(new_fd, &used_fds);
                            fd_cloexec(new_fd);
                            max_fd = max(max_fd, new_fd);
                        }

                        /* insufficient resources */
                        else
                        {
                            fprintf(stderr,
                                "uh_client_add(): Cannot allocate memory\n");

                            close(new_fd);
                        }
                    }
                }

                /* is a client socket */
                else
                {
                    if( 0)//! (cl = uh_client_lookup(cur_fd)) )
                    {
                        /* this should not happen! */
                        fprintf(stderr,
                            "uh_client_lookup(): No entry for fd %i!\n",
                            cur_fd);

                        goto cleanup;
                    }
		    cl = uh_client_add(new_fd, uh_listener_lookup(cur_fd));
#ifdef AEI_UHTTPD_REALM_AUTH
                    ret = getsockname(cur_fd, (struct sockaddr *)&serv_addr, &serv_len);
                    if(ret == 0)
                    {
                        serv_port = ntohs(serv_addr.sin_port);
                        int i;
                        protocol = NULL;
                        for (i = 0; i < https_rec.count; i ++)
                        {
                            if(serv_port == *(https_rec.https_port+i))
                            {
                                protocol = "https";
                                break;
                            }
                        }
                        if(NULL == protocol)
                            protocol = "http";
                    }
#endif
                    /* parse message header */
                    if( (req = uh_http_header_recv(cl)) != NULL )
                    {                   
                        /* RFC1918 filtering required? */
                        if( conf->rfc1918_filter &&
                            sa_rfc1918(&cl->peeraddr) &&
                            !sa_rfc1918(&cl->servaddr) )
                        {
                            uh_http_sendhf(cl, 403, "Forbidden",
                                "Rejected request from RFC1918 IP "
                                "to public server address");
                        }
                        else                    
                        {                           
#if HAVE_SESSION
                            snprintf(gWebParams.cli_addr,sizeof(gWebParams.cli_addr),"%s",get_cli_addr(&cl->peeraddr));
                            //gui_debug("gWebParams.cli_addr=%s",gWebParams.cli_addr);

                            int is_guest_ssid = 0;
                            
/*                          if(is_guest_ssid = dal_wifi_is_ssociated_guest_ssid(gWebParams.cli_addr))
                            {
                              gui_debug("is_guest_ssid=%d",is_guest_ssid);
                              uh_http_sendhf(cl, 404, "Not Found","No such file or directory");
                              goto session_login;
                            }
*/

                           int is_need_check_session = session_is_needcheck(req->url);
                            //is_need_check_session = 0;
                            unsigned int session_id = 1;
                            gui_debug("@@req->url=%s, is_need_check_session=%d", req->url, is_need_check_session);
                            if (is_need_check_session)
                            {
                                session_id = getSessionIDFromReq(req);
#ifdef AEI_UHTTPD_REALM_AUTH
                                access_realm = getAccessRealm(protocol, serv_port);
#endif
                                gui_debug("session_id=%u", session_id);
                                if (session_id == 1)
                                {
                                    gWebParams.wcb_main_login_success=0;                                
                                    if (strstr(req->url, UH_MAIN_PAGE) && session_is_logined(g_logined_session_id))
                                    {
                                        uh_http_sendf(cl, NULL,
                                            "HTTP/1.1 302 Found\r\n"
                                            "Location: %s?k=3\r\n"
                                            "Connection: close\r\n\r\n",
                                                UH_LOGIN_PAGE
                                        );
                                        char log_buff[BUFLEN_512] ={0};
                                        snprintf(log_buff, sizeof(log_buff), "from address:%s", gWebParams.cli_addr);
//                                        dal_write_log(OMT_LOG_INFO, LOG_SECURITY, "Firewall Info", LOG_E_T_USER_LOGIN_SESSION_TIMEOUT, log_buff);
                                        gui_debug("session disabled.");
                                        session_clear_single(g_logined_session_id);
                                        goto session_login;
                                    }
                                    else
                                    {
                                        uh_http_sendf(cl, NULL,
                                            "HTTP/1.1 302 Found\r\n"
                                            "Location: %s\r\n"
                                            "Connection: close\r\n\r\n",
                                                UH_LOGIN_PAGE
                                        );
                                        
                                        gui_debug("goto session_login");
                                        goto session_login;
                                    }
                                }
                                else
                                {
                                    if (session_is_logined(session_id))
                                    {
                                        req->session_id = session_id;
#ifdef AEI_UHTTPD_REALM_AUTH
                                        int level = 0;
                                        int checked = get_session_realm2id(session_id, access_realm, &level);
                                        if(checked != 0)
                                        {
                                            int ret = check_user_realm(gWebParams.login_username, gWebParams.login_password, access_realm, &level);
                                            if(ret == 0)
                                                set_session_realm2id(session_id, access_realm, level);
                                            else
                                            {
                                                gWebParams.wcb_main_login_success=0;
                                                uh_http_sendf(cl, NULL,
                                                    "HTTP/1.1 302 Found\r\n"
                                                    "Location: %s\r\n"
                                                    "Connection: close\r\n\r\n",
                                                        UH_LOGIN_PAGE
                                                );
                                                gui_debug("goto session_login");
                                                goto session_login;
                                            }
                                        }
                                        gui_debug("######### user=%s, level=%d ########\n", gWebParams.login_username, level);
                                        int has_access = isAllowAccess(req->url, level);
                                        if(has_access != 0)
                                        {
                                            gWebParams.wcb_main_login_success=0;
                                            uh_http_sendf(cl, NULL,
                                                "HTTP/1.1 302 Found\r\n"
                                                "Location: %s\r\n"
                                                "Connection: close\r\n\r\n",
                                                    UH_LOGIN_PAGE
                                            );
                                            gui_debug("goto session_login");
                                            goto session_login;
                                        }
#else
                                        //check access map for defferent user levels
                                        int has_access = isAllowAccess(req->url, gWebParams.login_username);
                                        gui_debug("has_access=%d",has_access);
                                        if (has_access == 0)
                                        {
                                            uh_http_sendf(cl, NULL,
                                                "HTTP/1.1 302 Found\r\n"
                                                "Location: %s\r\n"
                                                "Connection: close\r\n\r\n",
                                                    UH_MAIN_PAGE
                                            );
                                            
                                            gui_debug("access reject.");
                                            goto session_login;
                                        }
#endif
                                    }
                                    else
                                    {
                                        gWebParams.wcb_main_login_success=0;                              
                                        uh_http_sendf(cl, NULL,
                                            "HTTP/1.1 302 Found\r\n"
                                            "Location: %s\r\n"
                                            "Connection: close\r\n\r\n",
                                                UH_LOGIN_PAGE
                                        );
                                        
                                        gui_debug("goto session_login");
                                        goto session_login;
                                    }
                                }
                            }

                            else if (strstr(req->url, ".conf"))
                            {
                                gui_debug("req->url=%s", req->url);
                                uh_download_file(cl, req);
                            }                            
                            else
#endif
                            {
                                /* dispatch request */
                                if( (pin = uh_path_lookup(cl, req->url)) != NULL )
                                {
                                    /* auth ok? */
                                    if( !pin->redirected && uh_auth_check(cl, req, pin) )
                                    {
#if HAVE_SESSION
                                        //fake login
                                        if(safe_strcmp(req->url,"/") == 0)
                                        {
                                            uh_http_sendf(cl, NULL,
                                                "HTTP/1.1 302 Found\r\n"
                                                "Location: %s\r\n"
                                                "Connection: close\r\n\r\n",
                                                UH_MAIN_PAGE
                                            );
                                            goto session_login;
                                        }
#endif
                                        uh_dispatch_request(cl, req, pin);
                                    }
                                }
#if 0 //def AEI_OPENWRT sharko modify
                                else if (uh_hiddenpage_lookup(req->url))
                                {
                                    uh_file_hidden_request(cl, req);
                                }
#endif
                                /* 404 */
                                else
                                {
                                    /* Try to invoke an error handler */
                                    pin = uh_path_lookup(cl, conf->error_handler);
        
                                    if( pin && uh_auth_check(cl, req, pin) )
                                    {
                                        req->redirect_status = 404;
                                        uh_dispatch_request(cl, req, pin);
                                    }
                                    else
                                    {
                                        uh_http_sendhf(cl, 404, "Not Found",
                                            "No such file or directory");
                                    }
                                }
                            }
                        }
                    }
#ifdef HAVE_SESSION
                    session_login:
#endif
#ifdef HAVE_TLS
                    /* free client tls context */
                    if( conf->tls )
                        conf->tls_close(cl);
#endif

                    cleanup:

                    /* close client socket */
                    close(cur_fd);
                    FD_CLR(cur_fd, &used_fds);

                    /* remove from global client list */
                    uh_client_remove(cur_fd);
                }
            }
        }
	//break;
    //}

#ifdef HAVE_LUA
    /* destroy the Lua state */
    if( conf->lua_state != NULL )
        conf->lua_close(conf->lua_state);
#endif
}

#ifdef HAVE_TLS
static inline int uh_inittls(struct config *conf)
{
    /* library handle */
    void *lib;

    /* already loaded */
    if( conf->tls != NULL )
        return 0;

    /* load TLS plugin */
    if( ! (lib = dlopen("uhttpd_tls.so", RTLD_LAZY | RTLD_GLOBAL)) )
    {
        fprintf(stderr,
            "Notice: Unable to load TLS plugin - disabling SSL support! "
            "(Reason: %s)\n", dlerror()
        );

        return 1;
    }
    else
    {
        /* resolve functions */
        if( !(conf->tls_init   = dlsym(lib, "uh_tls_ctx_init"))      ||
            !(conf->tls_cert   = dlsym(lib, "uh_tls_ctx_cert"))      ||
            !(conf->tls_key    = dlsym(lib, "uh_tls_ctx_key"))       ||
            !(conf->tls_free   = dlsym(lib, "uh_tls_ctx_free"))      ||
            !(conf->tls_accept = dlsym(lib, "uh_tls_client_accept")) ||
            !(conf->tls_close  = dlsym(lib, "uh_tls_client_close"))  ||
            !(conf->tls_recv   = dlsym(lib, "uh_tls_client_recv"))   ||
            !(conf->tls_send   = dlsym(lib, "uh_tls_client_send"))
        ) {
            fprintf(stderr,
                "Error: Failed to lookup required symbols "
                "in TLS plugin: %s\n", dlerror()
            );
            exit(1);
        }

        /* init SSL context */
        if( ! (conf->tls = conf->tls_init()) )
        {
            fprintf(stderr, "Error: Failed to initalize SSL context\n");
            exit(1);
        }
    }

    return 0;
}
#endif

#ifdef AEI_REVJ
struct system_setting init_systemsetting()
{
    struct system_setting sys_setting;
    
    sys_setting.session_lifetime = UH_SESSION_LIFETIME;
    sys_setting.ipv4_http_port1 = UH_IPV4_HTTP_PORT1;
    sys_setting.ipv4_http_port2 = UH_IPV4_HTTP_PORT2;
    sys_setting.ipv4_https_port1 = UH_IPV4_HTTPS_PORT1;
    sys_setting.ipv4_https_port2 = UH_IPV4_HTTPS_PORT2;
    
    sys_setting.login_attempts = UH_LOGIN_ATTEMPTS;
    sys_setting.session_max = UH_SESSION_MAX;
    
    sprintf(sys_setting.current_user, "%s", UH_CURRENT_USER);

    return sys_setting;
}

void modify_system_setting(char *fieldname, int old_value, int new_value)
{
    char cmd[BUFLEN_256] = {0};
    sprintf(cmd, "sed -i 's/%s=%d/%s=%d/g' %s", fieldname, old_value, fieldname, new_value, UH_SYSTEM_SETTING_FILE);
    system(cmd);
    
}

void modify_system_setting_string(char *fieldname, char *new_value)
{
    char cmd[BUFLEN_256] = {0};
    sprintf(cmd, "sed -i 's/%s=.*/%s=%s/g' %s", fieldname, fieldname, new_value, UH_SYSTEM_SETTING_FILE);
    system(cmd);
}
#endif

int main (int argc, char **argv)
{
    /* master file descriptor list */
    fd_set serv_fds;

    /* working structs */
    struct sigaction sa;
    struct config conf;

    /* signal mask */
    sigset_t ss;

    /* maximum file descriptor number */
    int cur_fd, max_fd = 0;

#ifdef HAVE_TLS
    int tls = 0;
    int keys = 0;
#endif

    int bound = 0;
    int nofork = 1;

    /* args */
    int opt;
    char bind[128];
    char *port = NULL;

#ifdef AEI_REVJ
    /* http port number */
    int http_port_num = 0;
    int https_port_num = 0;
#endif

#ifdef HAVE_LUA
    /* library handle */
    void *lib;
#endif

    FD_ZERO(&serv_fds);


    /* parse args */
    memset(&conf, 0, sizeof(conf));
    memset(bind, 0, sizeof(bind));

    while( (opt = getopt(argc, argv,
        "fSDRC:K:E:I:p:s:h:c:l:L:d:r:m:x:i:t:T:A:")) > 0
    ) {
        switch(opt)
        {
            /* [addr:]port */
            case 'p':
            case 's':
#ifdef AEI_REVJ
                (opt == 'p') ? http_port_num++ : https_port_num++;
#endif
                if( (port = strrchr(optarg, ':')) != NULL )
                {
                    if( (optarg[0] == '[') && (port > optarg) && (port[-1] == ']') )
                    {
                        memcpy(bind, optarg + 1, min(sizeof(bind), (int)(port - optarg) - 2));
                        port++; 
                        gui_debug("ipv6: opt=%c, bind=%s, port=%s", opt, bind, port);
                    }
                    else
                    {
                        memcpy(bind, optarg, min(sizeof(bind), (int)(port - optarg)));
#ifdef AEI_REVJ
                        gui_debug("opt=%d, ipv4_http_port1=%d, ipv4_https_port1=%d", opt, g_sys_setting.ipv4_http_port1, g_sys_setting.ipv4_https_port1);
#else
                        port++;
#endif
                        gui_debug("ipv4: opt=%c,  bind=%s, port=%s", opt, bind, port);
                    }
                }
                else
                {
                    port = optarg;
                }

#ifdef HAVE_TLS
                if( opt == 's' )
                {
                    if( uh_inittls(&conf) )
                    {
                        fprintf(stderr,
                            "Notice: TLS support is disabled, "
                            "ignoring '-s %s'\n", optarg
                        );
                        continue;
                    }

                    tls = 1;
                }
#endif

#ifdef AEI_UHTTPD_REALM_AUTH
                if(opt == 's')
                {
                    char *tmp = (char *)realloc(https_rec.https_port, (https_rec.count+1) * sizeof(int));
                    if(tmp)
                    {
                        https_rec.https_port = tmp;
                        *(https_rec.https_port+https_rec.count) = atoi(port);
                        https_rec.count++;
                    }
                }
#endif

                memset(bind, 0, sizeof(bind));
                break;

#ifdef HAVE_TLS
            /* certificate */
            case 'C':
                if( !uh_inittls(&conf) )
                {
                    if( conf.tls_cert(conf.tls, optarg) < 1 )
                    {
                        fprintf(stderr,
                            "Error: Invalid certificate file given\n");
                        exit(1);
                    }

                    keys++;
                }

                break;

            /* key */
            case 'K':
                if( !uh_inittls(&conf) )
                {
                    if( conf.tls_key(conf.tls, optarg) < 1 )
                    {
                        fprintf(stderr,
                            "Error: Invalid private key file given\n");
                        exit(1);
                    }

                    keys++;
                }

                break;
#endif

            /* docroot */
            case 'h':
                if( ! realpath(optarg, conf.docroot) )
                {
                    fprintf(stderr, "Error: Invalid directory %s: %s\n",
                        optarg, strerror(errno));
                    exit(1);
                }
                break;

            /* error handler */
            case 'E':
                if( (strlen(optarg) == 0) || (optarg[0] == '/') )
                {
                    fprintf(stderr, "Error: Invalid error handler: %s\n",
                        optarg);
                    exit(1);
                }
                conf.error_handler = optarg;
                break;

            /* login file */
            case 'I':
                if( (strlen(optarg) == 0) || (optarg[0] == '/') )
                {
                    fprintf(stderr, "Error: Invalid login page: %s\n",
                        optarg);
                    exit(1);
                }
                conf.index_file = optarg;
                break;

            /* don't follow symlinks */
            case 'S':
                conf.no_symlinks = 1;
                break;

            /* don't list directories */
            case 'D':
                conf.no_dirlists = 1;
                break;

            case 'R':
                conf.rfc1918_filter = 1;
                break;

#ifdef HAVE_CGI
            /* cgi prefix */
            case 'x':
                conf.cgi_prefix = optarg;
                break;

            /* interpreter */
            case 'i':
                if( (optarg[0] == '.') && (port = strchr(optarg, '=')) )
                {
                    *port++ = 0;
                    uh_interpreter_add(optarg, port);
                }
                else
                {
                    fprintf(stderr, "Error: Invalid interpreter: %s\n",
                        optarg);
                    exit(1);
                }
                break;
#endif

#ifdef HAVE_LUA
            /* lua prefix */
            case 'l':
                conf.lua_prefix = optarg;
                break;

            /* lua handler */
            case 'L':
                conf.lua_handler = optarg;
                break;
#endif

#if defined(HAVE_CGI) || defined(HAVE_LUA)
            /* script timeout */
            case 't':
                conf.script_timeout = atoi(optarg);
                break;
#endif

            /* network timeout */
            case 'T':
                conf.network_timeout = atoi(optarg);
                break;

            /* tcp keep-alive */
            case 'A':
                conf.tcp_keepalive = atoi(optarg);
                break;

            /* no fork */
            case 'f':
                nofork = 1;
                break;

            /* urldecode */
            case 'd':
                if( (port = malloc(strlen(optarg)+1)) != NULL )
                {
                    /* "decode" plus to space to retain compat */
                    for (opt = 0; optarg[opt]; opt++)
                        if (optarg[opt] == '+')
                            optarg[opt] = ' ';

                    memset(port, 0, strlen(optarg)+1);
                    uh_urldecode(port, strlen(optarg), optarg, strlen(optarg));

                    printf("%s", port);
                    free(port);
                    exit(0);
                }
                break;

            /* basic auth realm */
            case 'r':
                conf.realm = optarg;
                break;

            /* md5 crypt */
            case 'm':
                //printf("%s\n", crypt(optarg, "$1$"));
                exit(0);
                break;

            /* config file */
            case 'c':
                conf.file = optarg;
                break;

            default:
                fprintf(stderr,
                    "Usage: %s -p [addr:]port [-h docroot]\n"
                    "   -f              Do not fork to background\n"
                    "   -c file         Configuration file, default is '/etc/httpd.conf'\n"
                    "   -p [addr:]port  Bind to specified address and port, multiple allowed\n"
#ifdef HAVE_TLS
                    "   -s [addr:]port  Like -p but provide HTTPS on this port\n"
                    "   -C file         ASN.1 server certificate file\n"
                    "   -K file         ASN.1 server private key file\n"
#endif
                    "   -h directory    Specify the document root, default is '.'\n"
                    "   -E string       Use given virtual URL as 404 error handler\n"
                    "   -I string       Use given filename as index page for directories\n"
                    "   -S              Do not follow symbolic links outside of the docroot\n"
                    "   -D              Do not allow directory listings, send 403 instead\n"
                    "   -R              Enable RFC1918 filter\n"
#ifdef HAVE_LUA
                    "   -l string       URL prefix for Lua handler, default is '/lua'\n"
                    "   -L file         Lua handler script, omit to disable Lua\n"
#endif
#ifdef HAVE_CGI
                    "   -x string       URL prefix for CGI handler, default is '/cgi-bin'\n"
                    "   -i .ext=path    Use interpreter at path for files with the given extension\n"
#endif
#if defined(HAVE_CGI) || defined(HAVE_LUA)
                    "   -t seconds      CGI and Lua script timeout in seconds, default is 60\n"
#endif
                    "   -T seconds      Network timeout in seconds, default is 30\n"
                    "   -d string       URL decode given string\n"
                    "   -r string       Specify basic auth realm\n"
                    "   -m string       MD5 crypt given string\n"
                    "\n", argv[0]
                );

                exit(1);
        }
    }

#ifdef HAVE_TLS
    if( (tls == 1) && (keys < 2) )
    {
        fprintf(stderr, "Error: Missing private key or certificate file\n");
        exit(1);
    }
#endif
    bound = 1;

    if( bound < 1 )
    {
        fprintf(stderr, "Error: No sockets bound, unable to continue\n");
        exit(1);
    }

    /* default docroot */
    if( !conf.docroot[0] && !realpath(".", conf.docroot) )
    {
        fprintf(stderr, "Error: Can not determine default document root: %s\n",
            strerror(errno));
        exit(1);
    }

    /* default realm */
    if( ! conf.realm )
        conf.realm = "Protected Area";

    /* config file */
    uh_config_parse(&conf);

    /* default network timeout */
    if( conf.network_timeout <= 0 )
        conf.network_timeout = 30;

#if defined(HAVE_CGI) || defined(HAVE_LUA)
    /* default script timeout */
    if( conf.script_timeout <= 0 )
        conf.script_timeout = 60;
#endif

#ifdef HAVE_CGI
    /* default cgi prefix */
    if( ! conf.cgi_prefix )
        conf.cgi_prefix = "/cgi-bin";
#endif

#ifdef AEI_BCM_FIRMWARE_UPLOAD
    //init_uhttpd_handle();
#endif
#ifdef HAVE_LUA
    /* load Lua plugin */
    if( ! (lib = dlopen("uhttpd_lua.so", RTLD_LAZY | RTLD_GLOBAL)) )
    {
        fprintf(stderr,
            "Notice: Unable to load Lua plugin - disabling Lua support! "
            "(Reason: %s)\n", dlerror()
        );
    }
    else
    {
        /* resolve functions */
        if( !(conf.lua_init    = dlsym(lib, "uh_lua_init"))    ||
            !(conf.lua_close   = dlsym(lib, "uh_lua_close"))   ||
            !(conf.lua_request = dlsym(lib, "uh_lua_request"))
        ) {
            fprintf(stderr,
                "Error: Failed to lookup required symbols "
                "in Lua plugin: %s\n", dlerror()
            );
            exit(1);
        }

        /* init Lua runtime if handler is specified */
        if( conf.lua_handler )
        {
            /* default lua prefix */
            if( ! conf.lua_prefix )
                conf.lua_prefix = "/lua";

            conf.lua_state = conf.lua_init(conf.lua_handler);
        }
    }
#endif

    /* fork (if not disabled) */
    if( ! nofork )
    {
        switch( fork() )
        {
            case -1:
                perror("fork()");
                exit(1);

            case 0:
                /* daemon setup */
                if( chdir("/") )
                    perror("chdir()");

                if( (cur_fd = open("/dev/null", O_WRONLY)) > -1 )
                    dup2(cur_fd, 0);

                if( (cur_fd = open("/dev/null", O_RDONLY)) > -1 )
                    dup2(cur_fd, 1);

                break;

            default:
                exit(0);
        }
    }

#ifdef HAVE_SESSION
    session_init();
    g_logined_session_id = 0;
#endif

    /*Check if the data center is ready, only when the
    data center is ready, then can process the HTTP request*/

/*
    while (1)
    {
        FILE *fp = NULL;
        fp = fopen("/tmp/datacenterIsReady", "r");
        if (fp)
        {
            fclose(fp);
            break;
        }
        sleep(10);
    }
*/
    /* server main loop */
    uh_mainloop(&conf, serv_fds, max_fd);

#ifdef HAVE_LUA
    /* destroy the Lua state */
    if( conf.lua_state != NULL )
        conf.lua_close(conf.lua_state);
#endif

    return 0;
}
