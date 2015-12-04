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
int
sws_cgi_request_handler(int fd_conn, st_request *request, st_opts_props *sop, char *client_ip_addr)
{
	sws_request_meta_variables_t	request_meta_vars;
	int								fd[2];
	pid_t							pid;
	char							line[MAX_LINE];
	int								nread, n;
	const char						*exec_err = "EXEC ERROR";
	size_t							exec_err_len;
	int								read_time;
	char							response_buf[MAX_LINE];
	char							gtime[50];
	time_t							t;
	struct tm						*time_gmt;

	exec_err_len = strlen(exec_err);
	read_time = 0;

	sws_cgi_parse_meta_vars(request, sop, client_ip_addr, &request_meta_vars);

	if (request_meta_vars.script_filename == NULL) {
		return 404;
	}

	/* set env vars */
	sws_cgi_set_env(&request_meta_vars);

	/* fork a new process to run cgi script, using pipe for communication */
	if (pipe(fd) < 0) {
		return 500;
	}

	if ((pid = fork()) < 0) {
		return 500;
	}
	else if (pid > 0) { /* parent */
		close(fd[1]);

		while ((nread = read(fd[0], line, MAX_LINE)) > 0) {
			++read_time;
			if (read_time == 1) {
				/* exec cgi script failure */
				if (strncmp(line, exec_err, exec_err_len) == 0) {
					return 500;
				}
				/* write response line the several headers */
				t = time(NULL);
				time_gmt = gmtime(&t);
				strftime(gtime, 50, "%a, %d %b %Y %T GMT", time_gmt);

				n = sprintf(response_buf, "HTTP/1.0 200 OK\r\nDate: %s\r\nServer: %s\r\n", gtime, request_meta_vars.server_software);
				if (n < 0) {
					return 500;
				}
				write(fd_conn, response_buf, n);
			}
			write(fd_conn, line, nread);
		}

		/* avoid zombie child process */
		waitpid(pid, NULL, 0);
	}
	else {				/* child */
		/* connent standard output to pipes */ 
		close(fd[0]);
		if (fd[1] != STDOUT_FILENO) {
			dup2(fd[1], STDOUT_FILENO);
			close(fd[1]);
		}

		if (execl(request_meta_vars.script_filename, request_meta_vars.script_filename, (char *)0) < 0) {
			write(STDOUT_FILENO, exec_err, exec_err_len);
			exit(0);
		}
	}
	return 200;
}

void
sws_cgi_debug(st_opts_props *sop) 
{
	st_request request;
	request.req_path = "/cgi-bin?";
	request.req_type = "GET";

	sws_cgi_request_handler(0, &request, sop, "192.168.1.5");
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
	setenv("QUERY_STRING", request_meta_vars->query_string != NULL ? request_meta_vars->query_string : "", 1);
	setenv("SCRIPT_FILENAME", request_meta_vars->script_filename, 1);
	if (request_meta_vars->path_info != NULL) {
		setenv("PATH_INFO", request_meta_vars->path_info, 1);
	}
	else {
		unsetenv("PATH_INFO");
	}
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
		strncpy(buffer + index, query_string_begin + 1, size);
		buffer[index + size] = '\0';
		++size;
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

	cur = begin + 1;
	while (cur < tail) {
		while (cur < tail && *cur != '/') {
			++cur;
		}
		if (sws_cgi_check_script(sop->cgi_dir, begin, cur - begin, script_path_buf)) {
			is_find_executable = 1;
			break;
		}
		++cur;
	}

	if (!is_find_executable) {
		request_meta_vars->script_name = NULL;
		request_meta_vars->script_filename = NULL;
		request_meta_vars->path_info = NULL;
	}
	else {
		index += size;
		sws_cgi_path_combine(cgi_prefix, cgi_prefix_size, 
					begin, cur - begin, buffer + index);
		size = strlen(buffer + index);
		++size;
		request_meta_vars->script_name = buffer + index;

		if (cur != tail) {
			index += size;
			size = tail - cur;
			strncpy(buffer + index, cur, size);
			buffer[index + size] = '\0';
			++size;
			request_meta_vars->path_info = buffer + index;
		}

		index += size;
		size = strlen(script_path_buf);
		strncpy(buffer + index, script_path_buf, size);
		buffer[index + size] = '\0';
		++size;
		request_meta_vars->script_filename = buffer + index;
	}
}

/* return 1 for executable, 0 for others */
static int
sws_cgi_check_script(char *cgi_root, char *script_path, size_t script_path_size, char *buf)
{
	struct stat	stat_buf;

	sws_cgi_path_combine(cgi_root, strlen(cgi_root), script_path, script_path_size, buf);
	
	if (access(buf, X_OK) == -1) {
		return 0;
	}
	else {
		if (lstat(buf, &stat_buf) < 0) {
			return 0;
		}
		if (S_ISREG(stat_buf.st_mode)) {
			return 1;
		}
		return 0;
	}
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

