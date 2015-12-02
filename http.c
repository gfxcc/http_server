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
    char* gtime = malloc(25 * sizeof(char));
    time_t t = time(NULL);
    struct tm *time_gmt;
    time_gmt = gmtime(&t);
    strcpy(gtime, asctime(time_gmt));
    return gtime;
}

char* sws_get_mtime(time_t t)
{
    char* mtime = malloc(25 * sizeof(char));
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

int sws_http_request_handler(char* client_request_line, st_opts_props *sop,
                     st_request *request, st_header *header, st_log *log)
{
    int status_code = 500;
    struct stat st_file, st_erro;
    char file[PATH_MAX]; bzero(file, PATH_MAX);
    char erro[PATH_MAX]; bzero(erro, PATH_MAX);
    strcpy(file, sop->root); strcat(file, "/");
    strcat(file, request->req_path);
    getcwd(erro, PATH_MAX); strcat(erro, "/dir_website");
    
    if (strncmp(request->type_conn, "HTTP/1.0", 8) != 0)
    {
        status_code = 405;
        strcat(erro, "/405.html");
        lstat(erro, &st_erro);
        header->time_last_mod = sws_get_mtime(st_erro.st_mtime);
        header->content_length = st_erro.st_size;
        log->req = request->req_string;
        log->http_status = status_code;
        log->resp_len = st_erro.st_size;
        return status_code;
    }
    
    else if (request->req_code == 0)
    {
        status_code = 400;
        strcat(erro, "/400.html");
        lstat(erro, &st_erro);
        header->time_last_mod = sws_get_mtime(st_erro.st_mtime);
        header->content_length = st_erro.st_size;
        log->req = request->req_string;
        log->http_status = status_code;
        log->resp_len = st_erro.st_size;
        return status_code;
    }
    
    else if (lstat(file, &st_file) == ENOENT)
    {
        status_code = 404;
        strcat(erro, "/404.html");
        lstat(erro, &st_erro);
        header->time_last_mod = sws_get_mtime(st_erro.st_mtime);
        header->content_length = st_erro.st_size;
        log->req = request->req_string;
        log->http_status = status_code;
        log->resp_len = st_erro.st_size;
        return status_code;
    }
    
    else if (S_IROTH & st_file.st_mode)
    {
        if (S_ISDIR(st_file.st_mode))
        {
            char index[PATH_MAX];
            struct stat st_index;
            sprintf(index, "%s/index.html", file);
            if (lstat(index, &st_index) != ENOENT)
            {
                if (S_IROTH & st_index.st_mode)
                {
                    status_code = 200; strcpy(file, index);
                    header->time_last_mod = sws_get_mtime(st_file.st_mtime);
                    header->content_length = st_file.st_size;
                    log->req = request->req_string;
                    log->http_status = status_code;
                    log->resp_len = st_file.st_size;
                    return status_code;
                }
                else
                {
                    status_code = 403;
                    strcat(erro, "/403.html");
                    lstat(erro, &st_erro);
                    header->time_last_mod = sws_get_mtime(st_erro.st_mtime);
                    header->content_length = st_erro.st_size;
                    log->req = request->req_string;
                    log->http_status = status_code;
                    log->resp_len = st_erro.st_size;
                    return status_code;
                }
            }
            else
            {
                status_code = 200;
                header->content_type = "Directory";
                header->time_last_mod = sws_get_mtime(st_file.st_mtime);
                header->content_length = st_file.st_size;
                log->req = request->req_string;
                log->http_status = status_code;
                log->resp_len = st_file.st_size;
                return status_code;
            }
        }
        else
        {
            status_code = 200;
            header->time_last_mod = sws_get_mtime(st_file.st_mtime);
            header->content_length = st_file.st_size;
            log->req = request->req_string;
            log->http_status = status_code;
            log->resp_len = st_file.st_size;
            return status_code;
        }
    }
    else
    {
        status_code = 403;
        strcat(erro, "/403.html");
        if(lstat(erro, &st_erro) == ENOENT)
        {   status_code = 500;  return status_code; }
        header->time_last_mod = sws_get_mtime(st_erro.st_mtime);
        header->content_length = st_erro.st_size;
        log->req = request->req_string;
        log->http_status = status_code;
        log->resp_len = st_erro.st_size;
        return status_code;
    }
}

void sws_http_respond_handler(int fd_connection, char* client_request_line, char* client_ip_addr, st_opts_props *sop)
{
    int status_code = 500;
    char response[MAX_BUFFER_LEN]; bzero(response, MAX_BUFFER_LEN);
    char erro[PATH_MAX]; bzero(erro, PATH_MAX);
    getcwd(erro, PATH_MAX); strcat(erro, "/dir_website");

    st_request *request = malloc(sizeof(st_request));
    st_header *header = malloc(sizeof(st_header));
    st_log *log = malloc(sizeof(st_log));
    
    sws_clireq_init(request);
    sws_header_init(header);
    sws_log_init(log, client_ip_addr);
    sws_server_parseline(client_request_line, request);
    status_code = sws_http_request_handler(client_request_line, sop, request, header, log);
    
    if (status_code != 500)
    {
        sprintf(response,
                "HTTP/1.0 %s Date: %s Server: %s Last-Modified: %s Content-Type: %s Content-Length: %lld\r\n\r\n",
                sws_get_http_status(log->http_status), header->time_now, header->server_name,
                header->time_last_mod, header->content_type, header->content_length);
        while(write(fd_connection, response, MAX_BUFFER_LEN) < 0);
        filelog_record(sop, log, erro);
    }
    else
    {
        struct stat st_err;
        char sws_err[PATH_MAX]; bzero(sws_err, PATH_MAX);
        getcwd(sws_err, PATH_MAX); strcat(sws_err, "/dir_website/500.html");
        lstat(sws_err, &st_err);
        log->ip_addr = client_ip_addr;
        log->time = sws_get_request_time();
        log->req = client_request_line;
        log->http_status = 500;
        log->resp_len = st_err.st_size;
        sprintf(response,
                "HTTP/1.0 %s \r\n\
                Date: %s \r\n\
                Server: SWS/1.0 \r\n\
                Last-Modified: %s \r\n\
                Content-Type: text/html \r\n\
                Content-Length: %lld\r\n\r\n",
                sws_get_http_status(log->http_status), log->time,
                sws_get_mtime(st_err.st_mtime), st_err.st_size);
        while(write(fd_connection, response, MAX_BUFFER_LEN) < 0);
        filelog_record(sop, log, erro);
    }
    free(request); free(header); free(log);
}