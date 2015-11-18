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

void sws_header_init(st_header *header)
{
    header->time_now = (char*)sws_malloc(sizeof(char));
    header->server_name = (char*)sws_malloc(sizeof(char));
    header->time_last_mod = (char*)sws_malloc(sizeof(char));
    header->content_type = (char*)sws_malloc(sizeof(char));
    
    header->time_now = sws_get_request_time();
    header->server_name = "sws ver alpha\r\n";
    header->time_last_mod = NULL;
    header->content_type = "text/html\r\n";
    header->content_length = 0;
}

void sws_clireq_init(st_request *request)
{
    request->req_path = (char*)sws_malloc(sizeof(char));
    request->req_type = (char*)sws_malloc(sizeof(char));
    request->req_string = (char*)sws_malloc(sizeof(char));
    request->type_conn = (char*)sws_malloc(sizeof(char));
    
    request->req_code = 0;
    request->req_path = NULL;
    request->req_type = NULL;
    request->req_string = NULL;
    request->type_conn = NULL;
}

void sws_log_init(st_log *log, char* client_ip_addr)
{
    log->ip_addr = (char*)sws_malloc(sizeof(char));
    log->time = (char*)sws_malloc(sizeof(char));
    log->req = (char*)sws_malloc(sizeof(char));
    
    log->ip_addr = client_ip_addr;
    log->time = sws_get_request_time();
    log->req = NULL;
    log->http_status = 0;
    log->resp_len = 0;
}

char* sws_get_http_status(int status_code)
{
    char* http_status = NULL;
    switch (status_code)
    {
        case 200:
            http_status = "200 OK\r\n"; break;
        case 302:
            http_status = "302 Found\r\n"; break;
        case 304:
            http_status = "304 Not Modified\r\n"; break;
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
        case 501:
            http_status = "501 Not Implemented\r\n"; break;
        case 502:
            http_status = "502 Bad Gateway\r\n"; break;
        case 503:
            http_status = "503 Service Unavailable\r\n"; break;
        case 550:
            http_status = "550 Permission Denied\r\n"; break;
    }
    return http_status;
}

char* sws_get_request_time()
{
    time_t t = time(NULL);
    struct tm *time_gmt;
    time_gmt = gmtime(&t);
    return asctime(time_gmt);
}

char* sws_get_mtime(time_t t)
{
    struct tm *mtime_gmt;
    mtime_gmt = gmtime(&t);
    return asctime(mtime_gmt);
}


int sws_server_parseline(char* client_request_line, st_request *req)
{
    char line1[PATH_MAX]; strcpy(line1, client_request_line);
    char line2[PATH_MAX]; strcpy(line2, client_request_line);
    char req_type[4], req_path[PATH_MAX], type_conn[PATH_MAX];
    int k = 0;
    if (strlen(line1) > 1)
    {
        char *token, *save;
        token = strtok_r(line1, " ", &save);
        while (token != NULL)
        {   k++; token = strtok_r(NULL, " ", &save); }
    }
    if (k != 3)
    {   return 0;   }
    else
    {
        k = 0;
        char *token, *save;
        token = strtok_r(line2, " ", &save);
        while (token != NULL)
        {
            if (k == 0) strcpy(req_type, token);
            else if (k == 1) strcpy(req_path, token);
            else if (k == 2) strcpy(type_conn, token);
            k++; token = strtok_r(NULL, " ", &save);
        }
        if (strncmp(req_type, "GET", 3) == 0)
        {   req->req_code = 1;  req->req_type = "GET";  }
        else if (strncmp(req_type, "HEAD", 4) == 0)
        {   req->req_code = 2;  req->req_type = "HEAD"; }
        req->req_path = req_path;
        req->type_conn = type_conn;
        return 1;
    }
}

void sws_http_request_handler(int fd_connection, char* client_request_line,
                              char* client_ip_addr, st_opts_props *sop)
{
    struct stat st_dir, st_file;
    char home[PATH_MAX] = "/Users/ChenWei/Desktop/enull";
    char path[PATH_MAX], file[PATH_MAX];
    
    st_request *request = sws_malloc(sizeof(st_request));
    st_header *header = sws_malloc(sizeof(st_header));
    st_log *log = sws_malloc(sizeof(st_log));
    
    sws_clireq_init(request);
    sws_header_init(header);
    sws_log_init(log, client_ip_addr);
    
    while(sws_server_parseline(client_request_line, request) == 0);
    
    strcpy(path, home);
    strcat(path, request->req_path);
    sws_lstat(path, &st_dir);
    log->ip_addr = client_ip_addr;
    
    /* Error 405: Method not allowed */
    if (strncmp(request->type_conn, "HTTP/1.0", 8) != 0)
    {
        strcpy(file, "/Users/ChenWei/Desktop/enull/methna.html");
        sws_lstat(file, &st_file);
        header->time_now = sws_get_request_time();
        
        printf("now %s", header->time_now);
        header->time_last_mod = sws_get_mtime(st_file.st_mtime);
        printf("now %s", header->time_now);
        
        printf("");
        
        header->content_length = st_file.st_size;
        log->req = request->req_string;
        log->http_status = 405;
        log->resp_len = st_file.st_size;
        
    }
    
    /* Error 400: Bad Request */
    else if (request->req_code == 0)
    {
        printf("KKKKKKK\n");
        strcpy(file, "/Users/ChenWei/Desktop/enull/methna.html");
        sws_lstat(file, &st_file);
        strftime(header->time_last_mod, 32, "%m/%d/%Y %H:%M:%S", gmtime(&st_file.st_mtime));
        header->content_length = st_file.st_size;
        log->req = request->req_string;
        log->http_status = 400;
        log->resp_len = st_file.st_size;
    }
    
    /* Error 404: Not Found */
    else if (access(path, F_OK) == -1)
    {
        printf("KKKKKKK\n");
        strcpy(file, "/Users/ChenWei/Desktop/enull/error.html");
        sws_lstat(file, &st_file);
        strftime(header->time_last_mod, 32, "%m/%d/%Y %H:%M:%S", gmtime(&st_file.st_mtime));
        header->content_length = st_file.st_size;
        log->req = request->req_string;
        log->http_status = 404;
        log->resp_len = st_file.st_size;
    }
    
    /* Target is accessible */
    else if (S_IROTH & st_dir.st_mode)
    {
        printf("KKKKKKK\n");
        /* Error 302: Founded But Moved */
        if (S_ISLNK(st_dir.st_mode))
        {
            realpath(file, path);
            sws_lstat(file, &st_file);
            strftime(header->time_last_mod, 32, "%m/%d/%Y %H:%M:%S", gmtime(&st_file.st_mtime));
            header->content_length = st_file.st_size;
            log->req = request->req_string;
            log->http_status = 302;
            log->resp_len = st_file.st_size;
        }
        
        /* Target is accessible folder ... */
        else if (S_ISDIR(st_dir.st_mode))
        {
            char index_html[PATH_MAX];
            sprintf(index_html, "%s/index.html", path);
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
                    strftime(header->time_last_mod, 32, "%m/%d/%Y %H:%M:%S", gmtime(&st_file.st_mtime));
                    header->content_length = st_file.st_size;
                    log->req = request->req_string;
                    log->http_status = 200;
                    log->resp_len = st_file.st_size;
                }
                /* ... or not accessble */
                else
                {
                    strcpy(file, "/home/${USER}/sws/forbid.html");
                    sws_lstat(file, &st_file);
                    strftime(header->time_last_mod, 32, "%m/%d/%Y %H:%M:%S", gmtime(&st_file.st_mtime));
                    header->content_length = st_file.st_size;
                    log->req = request->req_string;
                    log->http_status = 403;
                    log->resp_len = st_file.st_size;
                }
            }
            /* ... or index.html not exist */
            else
            {
                strcpy(file, path);
                sws_lstat(file, &st_file);
                strftime(header->time_last_mod, 32, "%m/%d/%Y %H:%M:%S", gmtime(&st_file.st_mtime));
                header->content_length = st_file.st_size;
                log->req = request->req_string;
                log->http_status = 200;
                log->resp_len = st_file.st_size;
            }
        }
        /* Target is file ... */
        else
        {
            strcpy(file, path);
            sws_lstat(file, &st_file);
            strftime(header->time_last_mod, 32, "%m/%d/%Y %H:%M:%S", gmtime(&st_file.st_mtime));
            header->content_length = st_file.st_size;
            log->req = request->req_string;
            log->http_status = 200;
            log->resp_len = st_file.st_size;
        }
    }
    /* Target is not accessible */
    else
    {
        printf("KKKKKKK\n");
        strcpy(file, "/home/${USER}/sws/forbid.html");
        sws_lstat(file, &st_file);
        strftime(header->time_last_mod, 32, "%m/%d/%Y %H:%M:%S", gmtime(&st_file.st_mtime));
        header->content_length = st_file.st_size;
        log->req = request->req_string;
        log->http_status = 403;
        log->resp_len = st_file.st_size;
    }
    
    char response[MAX_BUFFER_LEN] = "";
    sprintf(response, "HTTP/1.0 %sDate: %sServer: %sLast-Modified: %sContent-Type: %sContent-Length: %lld\r\n",
            sws_get_http_status(log->http_status), header->time_now, header->server_name,
            header->time_last_mod, header->content_type, header->content_length);
    
    sws_write(fd_connection, response, MAX_BUFFER_LEN);
    filelog_record(sop, log);
    
    free(request); free(header); free(log);
    
    close(fd_connection);
}

