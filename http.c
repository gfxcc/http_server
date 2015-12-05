/*
 *  http.c
 *  sws
 *
 *  Created by Chen Wei on 11/14/15.
 *  Copyright (C) 2015 Chen Wei. All rights reserved.
 */

/*  When server receives a request from client, it calls http_respond_handler
 *  so respond. In this method it uses server_parseline method to
 *  parse request line and save request type, uri, protocol in header. Then
 *  calls get_http_status to accquire status code according to header. With
 *  status code and request type, server will send header information and file
 *  content optionally.
 */


#include "http.h"
#include "server.h"
#include "filelog.h"
#include "magic_type.h"
#include "sws_define.h"
#include "cgi.h"
//char* modi_time=(char*)malloc(100*sizeof(char));
char modi_time[1000];
/* struct initial (done!) */
char*modified_time=" Sat, 05 Dec 2015 03:23:01 GMT";
void sws_header_init(st_header *header)
{
    header->time_now = sws_get_request_time();
    header->server_name = "SWS/1.0 \r\n";
    header->time_last_mod = NULL;
    header->content_type = "text/html";
    header->content_length = 0;
}

void sws_clireq_init(st_request *request)
{
    request->req_code = 0;
    request->req_path = NULL;
    request->req_type = NULL;
    request->req_string = NULL;
    request->type_conn = NULL;
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
            http_status = "200 OK\r\n";
            break;
        case 304:
            http_status = "304 Not Modified\r\n";
            break;
        case 400:
            http_status = "400 Bad Request\r\n";
            break;
        case 403:
            http_status = "403 Forbidden\r\n";
            break;
        case 404:
            http_status = "404 Not Found\r\n";
            break;
        case 500:
            http_status = "500 Internal Server Error\r\n";
            break;
        case 501:
            http_status = "501 Not implemented\r\n";
            break;
        default:
            break;
    }
    return http_status;
}

/* fetch time (done!) */
char* sws_get_request_time()
{
    char* gtime = malloc(50 * sizeof(char));
    time_t t = time(NULL);
    struct tm *time_gmt;
    time_gmt = gmtime(&t);
    //strcpy(gtime, asctime(time_gmt));
    strftime(gtime, 50, "%a, %d %b %Y %T GMT", time_gmt);
    return gtime;
}

char* sws_get_mtime(time_t t)
{
    char* mtime = malloc(50 * sizeof(char));
    struct tm *mtime_gmt;
    mtime_gmt = gmtime(&t);
    //strcpy(mtime, asctime(mtime_gmt));
    strftime(mtime, 50, "%a, %d %b %Y %T GMT", mtime_gmt);
    return mtime;
}


/* get request from client (done!)*/
void sws_server_parseline(char* client_request_line, st_request *req)
{
//    printf("%d\n",(int)strlen(modified_time));
    char *token, *token2, *str1, *str2;
    char *save;
    char*modify_time;
    char tmp[MAX_BUFFER_LEN];
    strcpy(tmp,client_request_line);
    char req_type[1024];
    int i = 0, j = 0;
    token = strtok_r(tmp, "\r\n", &str1);
    while(token != NULL)
    {
        if(i == 0)
        {
           save = token;
        }
        else
       {
            if(strncasecmp(token, "If-Modified-Since",
                  strlen("If-Modified-Since")) == 0)
            {
                str2 = token;
                strtok_r(str2,":",&modify_time);
                strcpy(modi_time,modify_time);
            }
        }
        token = strtok_r(NULL,"\r\n",&str1);
        i++;
    }
    token2 = strtok_r(NULL, " ", &save);
    while (token2 != NULL)
    {
        if (j == 0){
            strcpy(req_type, token2);
        }
        else if (j == 1){

            // get req_query
            req->req_query = malloc(strlen(token2) + 1);
            strcpy(req->req_query, token2);
            req->req_query[strlen(token2)] = '\0';

            for (int t = 0; t != (int)strlen(token2); t++)
            {
                if (token2[t] == '?')
                {
                    token2[t] = '\0';
                    break;
                }
            }
            req->req_path = malloc(strlen(token2));
            strcpy(req->req_path, token2);
        }
        else if (j == 2){
            req->type_conn = malloc(strlen(token2));
            strcpy(req->type_conn, token2);
        }
        token2 = strtok_r(NULL, " ", &save);
        j++;
    }

    req->req_string = client_request_line;
    /* judge head legal */
    if (j != 3)
    {
        req->req_code = -1;
        req->req_path = NULL;
    }
    else if (strncmp(req_type, "GET", 3) == 0 && i > 0){
        req->req_code = 1;  req->req_type = "GET";
    }
    else if (strncmp(req_type, "HEAD", 4) == 0 && i > 0){
        req->req_code = 2;  req->req_type = "HEAD";
    }
    else{
        req->req_type = req_type;
        req->req_code = 0;
    }

}

int sws_http_request_handler(char* client_request_line, st_opts_props *sop,
                     st_request *request, st_header *header, st_log *log , int *type)
{
    time_t last_mod;
    int status_code = 500;
    struct stat st_file, st_erro;
    char file[PATH_MAX];
    bzero(file, PATH_MAX);
    char erro[PATH_MAX];
    bzero(erro, PATH_MAX);
    if (request->req_path[1] == '~')
    {
        strcpy(file, getenv("HOME"));
        strcat(file, "/sws/");
        strcat(file, &request->req_path[2]);
        request->req_path = file;
    }
    else
    {
        strcpy(file, sop->root);
        strcat(file, "/");
        if (request->req_path)
        {
            strcat(file, request->req_path);
            request->req_path = file;
        }
    }
    getcwd(erro, PATH_MAX);
    strcat(erro, "/response_msg/");
    if (request->type_conn == NULL || request->req_code == -1)
    {
        status_code = 400;
        strcat(erro, "/400.html");
        lstat(erro, &st_erro);
        sws_http_status_msg(request,st_erro,status_code,header,log);
    }
    else if ( strncmp(request->type_conn, "HTTP/1.0", 8) != 0 )
    {
        status_code = 400;
        strcat(erro, "/400.html");
        lstat(erro, &st_erro);
        sws_http_status_msg(request,st_erro,status_code,header,log);
    }
    else if(request->req_code == 0)
    {
        status_code = 501;
        strcat(erro, "/501.html");
        lstat(erro, &st_erro);
        sws_http_status_msg(request,st_erro,status_code,header,log);
    }


    else{
        /* if file or dir doesnt exists*/
        if (stat(file, &st_file) == -1){
            if(errno == ENOENT){
                status_code = 404;
                strcat(erro, "/404.html");
                lstat(erro, &st_erro);
                sws_http_status_msg(request,st_erro,status_code,header,log);
            }
            else{
                status_code = 500;
                strcat(erro, "/500.html");
                lstat(erro, &st_erro);
                sws_http_status_msg(request,st_erro,status_code,header,log);
            }
        }
        /* file or dirt exists */
        else{
            /* if we can access the file*/
            if(S_IROTH & st_file.st_mode)
            {
                if(S_ISDIR(st_file.st_mode))
                {
                    char index[PATH_MAX];
                    struct stat st_index;
                    sprintf(index, "%s/index.html", file);
                    if (stat(index, &st_index) == -1)
                    {
                        /* no index.html, return directory */
                        if (errno == ENOENT)
                        {
                              /* if files's last modify time is smaller than if_modify_since then return 403 */
                              if(strlen(modi_time)==30 && parse_time(modi_time,&last_mod)&&
                                 last_mod>=st_file.st_mtime){
                                     status_code = 304;
                                     strcat(erro,"/304.html");
                                     lstat(erro,&st_erro);
                                     sws_http_status_msg(request,st_erro,status_code,header,log);
                              }
                              else
                              {
                                 status_code = 200;
                                 *type = 0;
                                 header->content_type=(char*)get_magictype(sop,file);
                                 sws_http_status_msg(request,st_file,status_code,header,log);
                              }

                        }
                        else
                        {
                            status_code = 500;
                            strcat(erro, "/500.html");
                            lstat(erro, &st_erro);
                            header->content_type=(char*)get_magictype(sop,file);
                            sws_http_status_msg(request,st_erro,status_code,header,log);
                        }
                    }
                    else
                    {   /* index.html */
                        if(strlen(modi_time)==30&& parse_time(modi_time,&last_mod)&&
                           last_mod>=st_file.st_mtime){
                               status_code = 304;
                               strcat(erro,"/304.html");
                               lstat(erro,&st_erro);
                               sws_http_status_msg(request,st_erro,status_code,header,log);
                        }
                        else
                        {

                             if(S_IROTH & st_index.st_mode){
                                  status_code = 200;
                                  *type = 1;
                                  sprintf(request->req_path, "%s", index);
                             }
                             else{
                                  status_code = 403;
                             }
                             header->content_type=(char*)get_magictype(sop,file);
                             sws_http_status_msg(request,st_index,status_code,header,log);
                       }
                    }
                }
                else{
                    /* file */
                    if(strlen(modi_time)==30 && parse_time(modi_time,&last_mod)&&
                       last_mod>=st_file.st_mtime){
                          status_code = 304;
                          strcat(erro,"/304.html");
                          lstat(erro,&st_erro);
                          sws_http_status_msg(request,st_erro,status_code,header,log);
                    }
                    else
                    {
                    status_code = 200;
                    *type = 1;
                    lstat(file, &st_file);
                    header->content_type=(char*)get_magictype(sop,file);
                    sws_http_status_msg(request,st_file,status_code,header,log);
                    }
                }
            }
            /* we cannot access the file*/
            else{
                status_code = 403;
                strcat(erro, "/403.html");
                lstat(erro, &st_erro);
                header->content_type=(char*)get_magictype(sop,file);
                sws_http_status_msg(request,st_erro,status_code,header,log);
            }
        }
    }
    return status_code;
}

void sws_http_status_msg(st_request *request, struct stat st,int status_code, st_header *header, st_log *log){
    header->time_last_mod = sws_get_mtime(st.st_mtime);
    header->content_length = st.st_size;
    log->req = request->req_string;
    log->http_status = status_code;
    log->resp_len = st.st_size;
}

/**
 * If error_500 is 1(true), which means there are some errors happen in server.c
 * In this case, just return 500 error.
 */
void sws_http_respond_handler(int fd_connection, char* client_request_line, char* client_ip_addr, st_opts_props *sop, int error_500)
{
    int status_code = 500;
    char response[MAX_BUFFER_LEN];
    bzero(response, MAX_BUFFER_LEN);
    char erro[PATH_MAX];
    bzero(erro, PATH_MAX);
    getcwd(erro, PATH_MAX);
    strcat(erro, "/response_msg");

    st_request *request = malloc(sizeof(st_request));
    st_header *header = malloc(sizeof(st_header));
    st_log *log = malloc(sizeof(st_log));
    int type=1;
    sws_clireq_init(request);
    sws_header_init(header);
    sws_log_init(log, client_ip_addr);

    /* 500 error from server.c */
    if(!error_500){
        sws_server_parseline(client_request_line, request);
        // judege CGI
        if (request->req_code == 1 && sop->cgi_dir != NULL && strncmp(request->req_path, "/cgi-bin", 8) == 0)
        {
            status_code = sws_cgi_request_handler(fd_connection, request,
						sop, client_ip_addr);
            if (status_code == 200)
                return;
        }
        else
        {
            status_code = sws_http_request_handler(client_request_line, sop, request, header, log, &type);
        }
    }
    else{
        char error[PATH_MAX];
        bzero(error,PATH_MAX);
        sprintf(error, "%s/%d.html",erro,status_code);
        struct stat st_erro;
        lstat(error, &st_erro);
        sws_http_status_msg(request,st_erro,status_code,header,log);
    }




    /* if status code ==  200 return content */
    if(status_code == 200){

        char *content = sws_getContent(request->req_path,type);
        sprintf(response,
                "HTTP/1.0 %sDate: %s\r\nServer: %sLast-Modified: %s\r\nContent-Type: %sContent-Length: %zu\r\n\r\n",
                sws_get_http_status(status_code), header->time_now, header->server_name,
                header->time_last_mod, header->content_type, strlen(content));
        if(strcmp(request->req_type,"GET")==0)
            sprintf(response,"%s%s",response,content);

        while(write(fd_connection, response, strlen(response)) < 0);
        filelog_record(sop, log, erro);
    }
    /* else return errors code */
    else{
        sprintf(erro, "%s/%d.html",erro,status_code);
        char *content = sws_getContent(erro,type);
        sprintf(response,
                "HTTP/1.0 %sDate: %s\r\nServer: %sLast-Modified: %s\r\nContent-Type: %sContent-Length: %zu\r\n\r\n",
                sws_get_http_status(status_code), header->time_now, header->server_name,
                header->time_last_mod, header->content_type, strlen(content));
        if(request->req_type!=NULL){
            if(strcmp(request->req_type,"GET")==0){
                sprintf(response,"%s%s",response,content);
            }
        }
        while(write(fd_connection, response, strlen(response)) < 0);
        filelog_record(sop, log, erro);
    }
    free(request); free(header); free(log);
}




