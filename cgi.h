/*
 *  cgi.h
 *  Copyright (C) Lingfei Hu
 */

#include <sys/utsname.h>

#include "sws.h"
#include "http.h"
#include "server.h"

#define MAX_LINE	1024

typedef struct {
	char *gateway_interface;
	char *path_info;
	char *query_string;
	char *remote_addr;
	char *request_method;
	char *script_name;
	char *script_filename;
	char *server_name;
	char *server_port;
	char *server_protocol;
	char *server_software;
	char buffer[MAX_BUFFER_LEN];
} sws_request_meta_variables_t;

int sws_cgi_request_handler(int fd_conn, st_request *request, st_opts_props *sop, char *client_ip_addr);
