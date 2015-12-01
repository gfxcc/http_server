//
//  http.h
//  sws
//
//  Created by Chen Wei on 11/14/15.
//  Copyright © 2015 Chen Wei. All rights reserved.
//

#ifndef http_h
#define http_h

#include "server.h"
#include "sws_define.h"

typedef struct http_header
{
    char* time_now;
    char* server_name;
    char* time_last_mod;
    char* content_type;
    off_t content_length;
}st_header;

typedef struct client_request
{
    int   req_code;
    char* req_type;
    char* req_path;
    char* req_string;
    char* type_conn;
}st_request;

typedef struct log_line
{
    char *ip_addr;
    char *time;
    char *req;
    int http_status;
    size_t resp_len;
}st_log;

char* sws_get_http_status(int status_code);
char* sws_get_request_time();
char* sws_get_mtime(time_t t);

void sws_server_parseline(char* client_request_line, st_request *req);

void sws_header_init(st_header *header);
void sws_clireq_init(st_request *request);
void sws_log_init(st_log *log, char* client_ip_addr);

void sws_http_request_handler(int fd_connection, char* client_request_line,
                              char* client_ip_addr, st_opts_props *sop);

#endif /* http_h */
