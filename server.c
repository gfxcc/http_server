//
//  server.c
//  sws
//
//  Created by Chen Wei on 11/14/15.
//  Copyright (C) 2015 Chen Wei. All rights reserved.
//

#include "http.h"
#include "server.h"
#include "filelog.h"
#include "sws_define.h"

void init_opts_props(st_opts_props *sop)
{
    sop->debug_mode=0;
    sop->cgi_dir = NULL;
    sop->ip_address = NULL;
    sop->file_log = NULL;
    sop->port = "8080";
}

/* terminate the child to avoid zombie process */
void sig_chld_handler(int signo)
{
    int stat;
    pid_t wpid;
    while((wpid = waitpid(-1, &stat, WNOHANG)) > 0);
}

// message handler function of server
// create socket, bind and listen the port
// while a new client connect has been accepted
// fork a child process to read the cmd line from client
// that supports a handing of  multi client connection
void server_exec(st_opts_props *sop)
{
    int fd_socket, fd_connection;
    const int on = 1;
    struct addrinfo hints, *res = NULL;
    struct sockaddr_storage ss_client;

    // hints is an addrinfo structure points that record
    // properties of the given socket address
    memset(&hints, 0, sizeof (hints));
    hints.ai_family = AF_UNSPEC;        //both IPv4 and IPv6 are supported
    hints.ai_protocol = IPPROTO_TCP;    //using TCP protocol
    hints.ai_socktype = SOCK_STREAM;    //specifies socktype as sock_stream
    hints.ai_flags = AI_PASSIVE;        //passive for bind of server socket

    // as a substitution of gethostbyname, getaddrinfo supports IPv6
    // given socket address structure returns by list point res
    sws_getaddrinfo(sop->ip_address, sop->port, &hints, &res);
    fd_socket = sws_socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    sws_setsockopt(fd_socket, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));
    sws_bind(fd_socket, res->ai_addr, res->ai_addrlen);

    if (!sop->debug_mode)
        sws_listen(fd_socket, MAX_BACK_LOG);
    else
        sws_listen(fd_socket, 1);

    //free res to prevent a memory leak
    freeaddrinfo(res);
    while(1)
    {
        pid_t pid;
        socklen_t sin_client_addr_size = sizeof(ss_client);
        // accept a new connected client
        fd_connection = sws_accept(fd_socket, (struct sockaddr*)&ss_client, &sin_client_addr_size);
        if (sop->debug_mode)
        {
            printf("Debug Mode Enabled \n");
            // fetch ip address of new client
            char client_ip_addr[INET6_ADDRSTRLEN], client_request[MAX_BUFFER_LEN];
                sws_getnameinfo((struct sockaddr*)&ss_client, sizeof(ss_client),
                                client_ip_addr, INET6_ADDRSTRLEN, 0, 0, NI_NUMERICHOST);
                printf("[Client] %s connected \n", client_ip_addr);
                char content[MAX_BUFFER_LEN];
                /* The nc's enter is different from telnet */
                char *enter_telnet = "\r\n\r\n";
                while (sws_read(fd_connection, client_request, MAX_BUFFER_LEN) > 0)
                {
                    if (strlen(client_request) > 0)
                    {
                        if(strlen(client_request)+strlen(content) <MAX_BUFFER_LEN)
                        {
                            strcat(content,client_request);
                            if(strlen(content) >= 4){
                                if(strncmp(content+strlen(content)-4,enter_telnet,4) == 0)
                                {
                                    sws_http_respond_handler(fd_connection, content, client_ip_addr, sop,0);
                                    bzero(content,MAX_BUFFER_LEN);
                                    break;
                                }
      			    }
                        }
                        else{
                            //call http 500
                            sws_http_respond_handler(fd_connection, content, client_ip_addr, sop,1);
                            break;
                        }
                    }
                    else{
                        // call http 500
                        sws_http_respond_handler(fd_connection, content, client_ip_addr, sop,1);
                        break;
                    }

                    bzero(client_request, MAX_BUFFER_LEN);
                }

                //sws_http_request_handler(fd_connection, client_ip_addr, sop);
                close(fd_connection);
                printf("[Client] %s diconnected \n", client_ip_addr);
        }
        else    // fork child process for each new accepted client
        {
            pid = sws_fork();
            // if child process
            if (pid == 0)
            {
                char client_ip_addr[INET6_ADDRSTRLEN], client_request[MAX_BUFFER_LEN];
                sws_getnameinfo((struct sockaddr*)&ss_client, sizeof(ss_client),
                                client_ip_addr, INET6_ADDRSTRLEN, 0, 0, NI_NUMERICHOST);
                printf("[Client] %s connected \n", client_ip_addr);
                char content[MAX_BUFFER_LEN];

                /* The nc's enter is different from telnet */
                char *enter_telnet = "\r\n\r\n";
                while (sws_read(fd_connection, client_request, MAX_BUFFER_LEN) > 0)
                {
                    if (strlen(client_request) > 0)
                    {
                        if(strlen(client_request)+strlen(content) < MAX_BUFFER_LEN)
                        {
                            strcat(content,client_request);
                            if(strlen(content) >= 4){
				if(strncmp(content+strlen(content)-4,enter_telnet,4) == 0)
                            	{
                                    sws_http_respond_handler(fd_connection, content, client_ip_addr, sop,0);
                                    bzero(content,MAX_BUFFER_LEN);
                                    break;
                                }
			    }
                        }
                        else{
                            //call http 500
                            sws_http_respond_handler(fd_connection, content, client_ip_addr, sop,1);
                            break;
                        }
                    }
                    else{
                        // call http 500
                        sws_http_respond_handler(fd_connection, content, client_ip_addr, sop,1);
                        break;
                    }

                    bzero(client_request, MAX_BUFFER_LEN);
                }
                //sws_http_request_handler(fd_connection, client_ip_addr, sop);
                close(fd_connection);
                printf("[Client] %s diconnected \n", client_ip_addr);
                exit(EXIT_SUCCESS);
            }
            else
                // terminate child to prevent a zombie process
                signal(SIGCHLD, sig_chld_handler);
            close(fd_connection);
        }
    }
}


