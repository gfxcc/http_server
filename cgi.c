/*
 *  cgi.c
 *  Copyright (C) Lingfei Hu
 */

#include "cgi.h"

static void sws_cgi_set_env(sws_request_meta_variables_t *request_meta_vars);
static void sws_cgi_parse_meta_vars(st_request *request, st_opts_props *sop, char *client_ip_addr, sws_request_meta_variables_t *request_meta_vars);
static int sws_cgi_check_script(char *cgi_root, char *script_path, size_t script_path_size, char *buf);
static void sws_cgi_path_combine(char *path1, size_t size1, char *path2, size_t size2, char *buf);

/* refer to APUE figure 15.6 */
void 
sws_cgi_resquest_handler(int fd_conn, st_request *request, st_opts_props *sop, char *client_ip_addr)
{
	sws_request_meta_variables_t	request_meta_vars;
	int								fd[2];
	pid_t							pid;
	char							line[MAX_LINE];
	int								nread;

	sws_cgi_parse_meta_vars(request, sop, client_ip_addr, &request_meta_vars);

	/* TODO: return 404 */
	if (request_meta_vars.script_filename == NULL) {
	}

	/* set env vars */
	sws_cgi_set_env(&request_meta_vars);

	/* fork a new process to run cgi script, using pipe for communication */
	if (pipe(fd) < 0) {
		/* TODO: 500 */
	}

	if ((pid = fork()) < 0) {
		/* TODO: 500 */
	}
	else if (pid > 0) { /* parent */
		close(fd[1]);
		
		while ((nread = read(fd[0], line, MAX_LINE)) > 0) {
			write(STDOUT_FILENO, line, nread);
		}

	}
	else {				/* child */
		/* connent standard input and output to pipes */ 
		/*
		if (fd[0] != STDIN_FILENO) {
			dup2(fd[0], STDIN_FILENO);
			close(fd[0]);
		}*/
		close(fd[0]);
		if (fd[1] != STDOUT_FILENO) {
			dup2(fd[1], STDOUT_FILENO);
			close(fd[1]);
		}

		if (execl(request_meta_vars.script_filename, request_meta_vars.script_filename, (char *)0) < 0) {
			/* return 500 */
		}
	}

}

static void
sws_cgi_set_env(sws_request_meta_variables_t *request_meta_vars)
{
	setenv("GATEWAY_INTERFACE", request_meta_vars->gateway_interface, 1);
	setenv("REMOTE_ADDR", request_meta_vars->remote_addr, 1);
	setenv("REQUEST_METHOD", request_meta_vars->request_method, 1);
	setenv("SERVER_NAME", request_meta_vars->server_name, 1);
	setenv("SERVER_PORT", request_meta_vars->server_port, 1);
	setenv("SERVER_PROTOCOL", request_meta_vars->server_protocol, 1);
	setenv("SERVER_SOFTWARE", request_meta_vars->server_software, 1);
	setenv("SCRIPT_NAME", request_meta_vars->script_name, 1);
	setenv("QUERY_STRING", request_meta_vars->query_string, 1);
	setenv("PATH_INFO", request_meta_vars->path_info, 1);
	setenv("SCRIPT_FILENAME", request_meta_vars->script_filename, 1);
}

static void
sws_cgi_parse_meta_vars(st_request *request, st_opts_props *sop, char *client_ip_addr, sws_request_meta_variables_t *request_meta_vars)
{
	int index;
	size_t size;
	const char *gateway_interface_str = "CGI/1.1";
	const char *server_protocol_str = "HTTP/1.0";
	const char *server_software_str = "SWS/1.0";
	char *server_name;
	char *buffer = request_meta_vars->buffer;

	memset(request_meta_vars, 0, sizeof(sws_request_meta_variables_t));

	/* GATEWAY_INTERFACE */
	index = 0; 
	size = strlen(gateway_interface_str);
	++size;
	strncpy(buffer + index, gateway_interface_str, size);
	request_meta_vars->gateway_interface = buffer + index;

	/* REMOTE_ADDR */
	index += size;
	size = strlen(client_ip_addr);
	++size;
	strncpy(buffer + index, client_ip_addr, size);
	request_meta_vars->remote_addr = buffer + index;

	/* REQUEST_METHOD */
	index += size;
	size = strlen(request->req_type);
	++size;
	strncpy(buffer + index, request->req_type, size);
	request_meta_vars->request_method = buffer + index;

	/* SERVER_NAME */
	if (sop->ip_address != NULL) {
		server_name = sop->ip_address;
	}
	else {
		server_name = "127.0.0.1";
	}
	index += size;
	size = strlen(server_name);
	++size;
	strncpy(buffer + index, server_name, size);
	request_meta_vars->server_name = buffer + index;

	/* SERVER_PORT */
	index += size;
	size = strlen(sop->port);
	++size;
	strncpy(buffer + index, sop->port, size);
	request_meta_vars->server_port = buffer + index;

	/* SERVER_PROTOCOL */
	index += size;
	size = strlen(server_protocol_str);
	++size;
	strncpy(buffer + index, server_protocol_str, size);
	request_meta_vars->server_protocol = buffer + index;

	/* SERVER_SOFTWARE */
	index += size;
	size = strlen(server_software_str);
	++size;
	strncpy(buffer + index, server_software_str, size);
	request_meta_vars->server_software = buffer + index;

	/* QUERY_STRING */
	char *path = request->req_path;
	size_t path_size = strlen(path);
	char *query_string_begin;
	query_string_begin = strchr(path, '?');
	if (query_string_begin != NULL) {
		index += size;
		size = path_size - (query_string_begin - path + 1);
		++size;
		strncpy(buffer + index, query_string_begin + 1, size);
		request_meta_vars->query_string = buffer + index;
	}
	else {
		request_meta_vars->query_string = NULL;
	}


	/* SCRIPT_NAME & PATH_INFO */
	char *cgi_prefix = "/cgi-bin";
	size_t cgi_prefix_size = strlen(cgi_prefix);
	char *begin, *tail, *cur;
	char script_path_buf[PATH_MAX];
	int is_find_executable = 0;

	begin = path + cgi_prefix_size;
	if (query_string_begin != NULL) {
		tail = query_string_begin;
	}
	else {
		tail = begin + path_size;
	}

	cur = begin;
	while (cur < tail) {
		while (cur < tail && *cur != '/') {
			++cur;
		}
		if (sws_cgi_check_script(sop->cgi_dir, begin, cur - begin, script_path_buf) == 0) {
			is_find_executable = 1;
			break;
		}
	}

	if (!is_find_executable) {
		request_meta_vars->script_name = NULL;
		request_meta_vars->script_filename = NULL;
		request_meta_vars->path_info = NULL;
	}
	else {
		index += size;
		sws_cgi_path_combine(cgi_prefix, cgi_prefix_size, 
					script_path_buf, strlen(script_path_buf), buffer + index);
		size = strlen(buffer + index);
		request_meta_vars->script_name = buffer + index;

		if (cur != tail) {
			index += size;
			size = tail - cur;
			strncpy(buffer + index, cur, size);
			request_meta_vars->path_info = buffer + index;
		}

		index += size;
		sws_cgi_path_combine(sop->cgi_dir, strlen(sop->cgi_dir), 
					script_path_buf, strlen(script_path_buf), buffer + index);
		size = strlen(buffer + index);
		request_meta_vars->script_filename = buffer + index;
	}
}

/* return 0 for executable, -1 for others */
static int
sws_cgi_check_script(char *cgi_root, char *script_path, size_t script_path_size, char *buf)
{
	sws_cgi_path_combine(cgi_root, strlen(cgi_root), script_path, script_path_size, buf);
	
	return access(buf, X_OK);
}

static void
sws_cgi_path_combine(char *path1, size_t size1, char *path2, size_t size2, char *buf)
{
	if (path1[size1 - 1] == '/') {
		--size1;
	}
	if (path2[0] == '/') {
		++path2;
		--size2;
	}

	memcpy(buf, path1, size1);
	buf[size1] = '/';
	memcpy(buf + size1 + 1, path2, size2);
	buf[size1 + size2 + 1] = '\0';
}

