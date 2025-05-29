/*
 * uhttpd - Tiny single-threaded httpd - Static file header
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

#ifndef _UHTTPD_FILE_

#include <fcntl.h>
#include <time.h>
#include <strings.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <linux/limits.h>

struct mimetype {
    const char *extn;
    const char *mime;
};

void uh_file_request(struct client *cl, struct http_request *req, struct path_info *pi);

#ifdef AEI_REVJ
void uh_download_file(struct client *cl, struct http_request *req);
void send_download_file(struct client *cl, struct http_request *req, char *filename, char *buf, int buflen);
#endif

#if 0 //def AEI_OPENWRT sharko modify
#define HIDDENPAGE_HTML "test.html"
#define HIDDENPAGE_XML "test.xml"

typedef char *(*HIDDENPAGE_FUNC)(void);
typedef struct {
    char *hiddenpagename;
    HIDDENPAGE_FUNC hiddenpagefunc;
} HIDDENPAGE_VAR;

int uh_hiddenpage_lookup(const char *url);
void uh_file_hidden_request(struct client *cl, struct http_request *req);
#endif

#endif
