//
//  redefine.h
//  sws
//
//  Created by Chen Wei on 11/13/15.
//  Copyright (C) 2015 Chen Wei. All rights reserved.
//

#ifndef redefine_h
#define redefine_h

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include <errno.h>
#include <signal.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <limits.h>
#include <semaphore.h>
#include <dirent.h>
#include <magic.h>

#define MAX_BACK_LOG        1024
#define MAX_CONTENT_LEN     8192
#define MAX_BUFFER_LEN      8192

int sws_socket(int family, int socktype, int protocol);
int sws_accept(int fd_socket, struct sockaddr *s_addr, socklen_t *s_len);
int sws_open(const char* pathname, int rcs, mode_t mode);
int sws_dup2(int oldfd, int newfd);
unsigned int sws_sleep(unsigned int second);

pid_t sws_fork(void);
pid_t sws_wait(int *status);
pid_t sws_waitpid(pid_t pid, int *status, int options);
sig_t sws_signal(int sig, sig_t sig_chld_handler);

ssize_t sws_read(int fd_conn, void *buf, size_t len);
ssize_t sws_write(int fd_conn, void *buf, size_t len);

/* get file at path content. file == 1 mean path indicate file; file == 0, path indicate directory.
 * return file list in the directory. Formate as: file.html\r\nfile2.html\r\nfile3.html\r\n      */
char* sws_getContent(char* path, int file);

void sws_stderror(const char* message);
void sws_getaddrinfo(const char *host, const char *serv, const struct addrinfo *hints, struct addrinfo **res);
void sws_setsockopt(int fd_socket, int level, int optname, const void *optval, socklen_t optlen);
void sws_getnameinfo(const struct sockaddr *s_addr, socklen_t s_len, char *host, socklen_t host_len, char *serv, socklen_t serv_len, int flags);
void sws_bind(int fd_socket, struct sockaddr *s_addr, socklen_t s_len);
void sws_listen(int fd_socket, int connections);
void sws_connect(int fd_socket, struct sockaddr *s_addr, socklen_t s_len);
void sws_inet_pton(int family, const char *src, void *dst);
void sws_close(int fd);
void sws_lstat(const char *path, struct stat *buf);
void sws_fstat(int fd, struct stat *buf);
void sws_daemon(int nochdir, int noclose);
void sws_pipe(int fd_pipe[2]);

void* sws_malloc(size_t size);

const char *inet_ntop(int family, const void *src, char *dst, socklen_t size);

#endif /* redefine_h */
