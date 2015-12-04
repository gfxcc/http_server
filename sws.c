//
//  main.c
//  sws
//
//  Created by Chen Wei on 11/13/15.
//  Copyright (C) 2015 Chen Wei. All rights reserved.
//

#include "sws.h"
#include "http.h"
#include "server.h"

void usage();
void root_parse(char* arg, st_opts_props* sop);

void root_parse(char* arg, st_opts_props* sop)
{
    printf("%s\n", arg);
    char root[PATH_MAX];
    char path[PATH_MAX]; strcpy(path, arg);
    int path_len = (int)strlen(path);
    if (path[0] != '\"' || path[path_len - 1] != '\"' || path_len < 3)
    {
	printf("%c, %c\n", path[0], path[path_len - 1]);
        fprintf(stderr, "Please pass the dir within quotes\n");
        exit(EXIT_FAILURE);
    }
    if ((path[1] == '~' && path_len == 3) || (path[1] == '~' && path[2] == '/'))
    {
        strcpy(root, getenv("HOME"));
        strncat(root, "/sws/", 5);
        if (path_len > 4){
            for (int i = 3; i < path_len - 1; i++)
            {   strncpy(root, &path[i], 1);  }
        }
    }
    else
    {
        for (int i = 1; i < path_len - 1; i++)
        {   strncpy(root, &path[i], 1); }
    }
    strncpy(root, "\0", 1);
    struct stat st_root;
    int rc = lstat(root, &st_root);
    if (rc == 0 && S_ISDIR(st_root.st_mode))
    {   sop->root = root;   }
    else{
        fprintf(stderr, "Dir %s is not exist\n", root);
        exit(EXIT_FAILURE);
    }
}

int main(int argc, char *argv[]) {

    /* initialization */
    st_opts_props server_props;
    init_opts_props(&server_props);
    
    /* option analysis */
    int opt = 0, port_to_int;
    while ((opt = getopt(argc, argv, "c:dhi:l:p:")) != -1)
    {
        switch (opt)
        {
            // cgi_dir options
            case 'c':
                server_props.cgi_dir = optarg;
                break;
                
            // debugging options
            case 'd':
                server_props.debug_mode = 1;
                break;
                
            // usage options
            case 'h':
                usage(); exit(EXIT_FAILURE);
		break;
                
            // specify server ip address options
            case 'i':
                server_props.ip_address = optarg;
                break;
                
            // log file options
            case 'l':
                server_props.file_log = optarg;
                break;
            
            // specify server port
            case 'p':
                port_to_int = atoi(optarg);
                // set the valid port range from 1024 to 65535 to prevent a
                // condition that port has been occupied as much as possible
                if(port_to_int >= 1024 && port_to_int <= 65535)
                    server_props.port = optarg;
                else
                    fprintf(stderr, "Valid port range: 1024 - 65535\n"),
                    exit(EXIT_FAILURE);
                break;
            
            // invalid arguments
            default:
                fprintf(stderr, "Invalid argument %c\n", opt);
                exit(EXIT_FAILURE);
                break;
        }
    }
    //argc -= optind;
    //argv += optind;

    root_parse(argv[optind], &server_props);
 
    /* Daemonize if -d not set */
    /*
    if (!server_props.debug_mode)
        sws_daemon(1, 0);
    */
    server_exec(&server_props);
    return EXIT_SUCCESS;
}

void usage()
{
    printf("Usage: \n");
    printf("/-------------------------------------------------------/\n");
    printf(" NAME:                                                   \n");
    printf("    sws --- a simple web server                          \n");
    printf(" SYNOPSIS:                                               \n");
    printf("    sws [-dh] [-c dir] [-i addr] [-l file] [-p port] dir \n");
    printf("/-------------------------------------------------------/\n");
}
