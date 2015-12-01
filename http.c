//
//  http.c
//  sws
//
//  Created by Chen Wei on 11/14/15.
//  Copyright © 2015 Chen Wei. All rights reserved.
//

#include "http.h"
#include "server.h"
#include "filelog.h"
#include "sws_define.h"

/* struct initial (done!) */
void sws_header_init(st_header *header)
{
    header->time_now = sws_get_request_time();
    header->server_name = "SWS/0.1 \r\n";
    header->time_last_mod = NULL;
    header->content_type = "text/html\r\n";
    header->content_length = 0;
}

void sws_clireq_init(st_request *request)
{
    request->req_code = 0;
    request->req_path = "/";
    request->req_type = NULL;
    request->req_string = NULL;
    request->type_conn = "HTTP/1.0";
}

void sws_log_init(st_log *log, char* client_ip_addr)
{
    log->ip_addr = client_ip_addr;
    log->time = sws_get_request_time();
    log->req = NULL;
    log->http_status = 0;
    log->resp_len = 0;
}

/* fetch stat string (done!) */
char* sws_get_http_status(int status_code)
{
    char* http_status = NULL;
    switch (status_code)
    {
        case 200:
            http_status = "200 OK\r\n"; break;
        case 400:
            http_status = "400 Bad Request\r\n"; break;
        case 403:
            http_status = "403 Forbidden\r\n"; break;
        case 404:
            http_status = "404 Not Found\r\n"; break;
        case 405:
            http_status = "405 Method Not Allowed\r\n"; break;
        case 500:
            http_status = "500 Internal Server Error\r\n"; break;
    }
    return http_status;
}

/* fetch time (done!) */
char* sws_get_request_time()
{
    char* gtime = sws_malloc(25 * sizeof(char));
    time_t t = time(NULL);
    struct tm *time_gmt;
    time_gmt = gmtime(&t);
    strcpy(gtime, asctime(time_gmt));
    return gtime;
}

char* sws_get_mtime(time_t t)
{
    char* mtime = sws_malloc(25 * sizeof(char));
    struct tm *mtime_gmt;
    mtime_gmt = gmtime(&t);
    strcpy(mtime, asctime(mtime_gmt));
    return mtime;
}

/* get request from client (done!)*/
void sws_server_parseline(char* client_request_line, st_request *req)
{
    char line[PATH_MAX]; strcpy(line, client_request_line);
    char req_type[4], req_path[PATH_MAX], type_conn[PATH_MAX];
    char *token, *save;
    int argc_reqline = 0;
    token = strtok_r(line, " ", &save);
    while (token != NULL)
    {
        if (argc_reqline == 0) strcpy(req_type, token);
        else if (argc_reqline == 1) strcpy(req_path, token);
        else if (argc_reqline == 2) strcpy(type_conn, token);
        token = strtok_r(NULL, " ", &save);
        argc_reqline += 1;
    }
    req->req_string = client_request_line;
    if (strncmp(req_type, "GET", 3) == 0 && argc_reqline > 0)
    {   req->req_code = 1;  req->req_type = "GET";  }
    else if (strncmp(req_type, "HEAD", 4) == 0 && argc_reqline > 0)
    {   req->req_code = 2;  req->req_type = "HEAD"; }
    else
    {   req->req_type = req_type;   }
    if (argc_reqline > 1) req->req_path = req_path;
    if (argc_reqline > 2) req->type_conn = type_conn;
    if (argc_reqline > 3) req->req_code = 0;
}

void sws_http_request_handler(int fd_connection, char* client_request_line,
                              char* client_ip_addr, st_opts_props *sop)
{
    struct stat st_file, st_erro;
    char file[PATH_MAX], erro[PATH_MAX];
    bzero(file, PATH_MAX); bzero(erro, PATH_MAX);
    strcpy(file, sop->root); strcat(file, "/");
    getcwd(erro, PATH_MAX); strcat(erro, "/dir_website");
    
    st_request *request = sws_malloc(sizeof(st_request));
    st_header *header = sws_malloc(sizeof(st_header));
    st_log *log = sws_malloc(sizeof(st_log));
    
    sws_clireq_init(request);
    sws_header_init(header);
    sws_log_init(log, client_ip_addr);
    sws_server_parseline(client_request_line, request);
    
    strcat(file, request->req_path);
    lstat(file, &st_file);
    
    /* Error 405: Method not allowed */
    if (strncmp(request->type_conn, "HTTP/1.0", 8) != 0)
    {
        strcat(erro, "/405.html");
        sws_lstat(erro, &st_erro);
        header->time_last_mod = sws_get_mtime(st_erro.st_mtime);
        header->content_length = st_erro.st_size;
        log->req = request->req_string;
        log->http_status = 405;
        log->resp_len = st_erro.st_size;
    }
    
    /* Error 400: Bad Request */
    else if (request->req_code == 0)
    {
        strcat(erro, "/400.html");
        sws_lstat(erro, &st_erro);
        header->time_last_mod = sws_get_mtime(st_erro.st_mtime);
        header->content_length = st_erro.st_size;
        log->req = request->req_string;
        log->http_status = 400;
        log->resp_len = st_erro.st_size;
    }
    
    /* Error 404: Not Found */
    else if (access(file, F_OK) == -1)
    {
        strcat(erro, "/404.html");
        sws_lstat(erro, &st_erro);
        header->time_last_mod = sws_get_mtime(st_erro.st_mtime);
        header->content_length = st_erro.st_size;
        log->req = request->req_string;
        log->http_status = 404;
        log->resp_len = st_erro.st_size;
    }
    
    /* Target is accessible */
    else if (S_IROTH & st_file.st_mode)
    {
        /* Target is accessible folder ... */
        if (S_ISDIR(st_file.st_mode))
        {
            char index_html[PATH_MAX];
            sprintf(index_html, "%s/index.html", file);
            
            /* ... with index.html exist ... */
            if (access(index_html, F_OK) == 0)
            {
                struct stat st_index;
                sws_lstat(index_html, &st_index);
                
                /* ... and accessble */
                if (S_IROTH & st_index.st_mode)
                {
                    strcpy(file, index_html);
                    sws_lstat(file, &st_file);
                    header->time_last_mod = sws_get_mtime(st_file.st_mtime);
                    header->content_length = st_file.st_size;
                    log->req = request->req_string;
                    log->http_status = 200;
                    log->resp_len = st_file.st_size;
                }
                
                /* ... or not accessble */
                else
                {
                    strcat(erro, "/403.html");
                    sws_lstat(erro, &st_erro);
                    header->time_last_mod = sws_get_mtime(st_erro.st_mtime);
                    header->content_length = st_erro.st_size;
                    log->req = request->req_string;
                    log->http_status = 403;
                    log->resp_len = st_erro.st_size;
                }
            }
            
            /* ... or index.html not exist */
            else
            {
                header->time_last_mod = sws_get_mtime(st_file.st_mtime);
                header->content_length = st_file.st_size;
                log->req = request->req_string;
                log->http_status = 200;
                log->resp_len = st_file.st_size;
            }
        }
        
        /* Target is file ... */
        else
        {
            if ((S_IROTH & st_file.st_mode))
            {
                header->time_last_mod = sws_get_mtime(st_file.st_mtime);
                header->content_length = st_file.st_size;
                log->req = request->req_string;
                log->http_status = 200;
                log->resp_len = st_file.st_size;
            }
            else
            {
                strcat(erro, "/403.html");
                sws_lstat(erro, &st_erro);
                header->time_last_mod = sws_get_mtime(st_erro.st_mtime);
                header->content_length = st_erro.st_size;
                log->req = request->req_string;
                log->http_status = 403;
                log->resp_len = st_erro.st_size;
            }
            
        }
    }
    
    /* Target is not accessible */
    else
    {
        strcat(erro, "/403.html");
        sws_lstat(erro, &st_erro);
        header->time_last_mod = sws_get_mtime(st_erro.st_mtime);
        header->content_length = st_erro.st_size;
        log->req = request->req_string;
        log->http_status = 403;
        log->resp_len = st_erro.st_size;
    }
    
    char response[MAX_BUFFER_LEN];
    bzero(response, MAX_BUFFER_LEN);
    sprintf(response,
            "HTTP/1.0 %sDate: %sServer: %sLast-Modified: %sContent-Type: %sContent-Length: %lld\r\n\r\n",
            sws_get_http_status(log->http_status), header->time_now, header->server_name,
            header->time_last_mod, header->content_type, header->content_length);
    
    sws_write(fd_connection, response, MAX_BUFFER_LEN);
    
    filelog_record(sop, log);
    free(request); free(header); free(log);
}

