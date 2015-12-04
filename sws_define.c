//
//  redefine.c
//  sws
//
//  Created by Chen Wei on 11/13/15.
//  Copyright (C) 2015 Chen Wei. All rights reserved.
//
#include "sws_define.h"

/* Re-define for error message generation */
void sws_stderror(const char* message)
{
    fprintf(stderr, "%s: %s\n", message, strerror(errno));
    exit(EXIT_FAILURE);
}

/* Re-define for socket interface */
int sws_socket(int family, int socktype, int protocol)
{
    int rc_socket;
    if ((rc_socket = socket(family, socktype, protocol)) == -1)
        sws_stderror("Fail to create socket");
    return rc_socket;
}

int sws_accept(int fd_socket, struct sockaddr *s_addr, socklen_t *s_len)
{
    int rc_accept;
    if ((rc_accept = accept(fd_socket, s_addr, s_len)) < 0)
        sws_stderror("Fail to accept");
    return rc_accept;
}

void sws_getaddrinfo(const char *host, const char *serv,
                     const struct addrinfo *hints, struct addrinfo **res)
{
    if (getaddrinfo(host, serv, hints, res) != 0)
        sws_stderror("Fail to set socket options");
}

void sws_setsockopt(int fd_socket, int level, int optname, const void *optval, socklen_t optlen)
{
    if (setsockopt(fd_socket, level, optname, optval, optlen) != 0)
        sws_stderror("Fail to set socket options");
}

void sws_getnameinfo(const struct sockaddr *s_addr, socklen_t s_len,
                     char *host, socklen_t host_len, char *serv, socklen_t serv_len, int flags)
{
    if (getnameinfo(s_addr, s_len, host, host_len, serv, serv_len, flags) != 0)
        sws_stderror("Fail to get client IP address");
}

void sws_bind(int fd_socket, struct sockaddr *s_addr, socklen_t s_len)
{
    if (bind(fd_socket, s_addr, s_len) < 0)
        sws_stderror("Fail to bind");
}

void sws_listen(int fd_socket, int connections)
{
    if (listen(fd_socket, connections) < 0)
        sws_stderror("Fail to bind");
}

void sws_connect(int fd_socket, struct sockaddr *s_addr, socklen_t s_len)
{
    if ((connect(fd_socket, s_addr, s_len)) < 0)
        sws_stderror("Fail to connect");
}

void sws_inet_pton(int family, const char *src, void *dst)
{
    int rc_ip;
    if ((rc_ip = inet_pton(family, src, dst)) <= 0)
        sws_stderror("Fail to convert IP address");
}

const char *inet_ntop(int family, const void *src, char *dst, socklen_t size)
{
    const char *rc_in;
    if ((rc_in = inet_ntop(family, src, dst, size)) == NULL)
        sws_stderror("Fail to convert IP address");
    return rc_in;
}

/* Re-define for I/O Control */

int sws_open(const char* pathname, int rcs, mode_t mode)
{
    int rc_open;
    if ((rc_open = open(pathname, rcs, mode)) < 0)
        sws_stderror("Fail to open");
    return rc_open;
}

void sws_close(int fd)
{
    int rc_close;
    if ((rc_close = close(fd)) < 0)
        sws_stderror("Fail to close");
}

void sws_lstat(const char *path, struct stat *buf)
{
    if (lstat(path, buf) < 0)
        sws_stderror("Fail to obtain stat");
}

void sws_fstat(int fd, struct stat *buf)
{
    if (fstat(fd, buf) < 0)
        sws_stderror("Fail to obtain file stat");
}

ssize_t sws_read(int fd_conn, void *buf, size_t len)
{
    ssize_t rc_read;
    if ((rc_read = read(fd_conn, buf, len)) < 0)
        sws_stderror("Fail to read");
    return rc_read;
}

ssize_t sws_write(int fd_conn, void *buf, size_t len)
{
    ssize_t rc_write;
    if ((rc_write = write(fd_conn, buf, len)) < 0)
        sws_stderror("Fail to write");
    return rc_write;
}

int sws_dup2(int oldfd, int newfd)
{
    int res_dup2;
    if ((res_dup2 = dup2(oldfd, newfd)) < 0)
        sws_stderror("Fail to dup file descriptor");
    return res_dup2;
}

void sws_pipe(int fd_pipe[2])
{
    if (pipe(fd_pipe) == -1)
        sws_stderror("Fail to create pipe");
}

/* Re-define for process control */

pid_t sws_fork(void)
{
    pid_t pid;
    if ((pid = fork()) < 0)
        sws_stderror("Fail to fork child");
    return pid;
}

pid_t sws_wait(int *status)
{
    pid_t wpid;
    if ((wpid = wait(status)) < 0)
        sws_stderror("Fail to wait");
    return wpid;
}

pid_t sws_waitpid(pid_t pid, int *status, int options)
{
    pid_t wpid;
    if ((wpid = waitpid(pid, status, options)) < 0)
        sws_stderror("Fail to waitpid");
    return wpid;
}

sig_t sws_signal(int sig, sig_t sig_chld_handler)
{
    sig_t signl;
    if((signl = signal(sig, sig_chld_handler)) == SIG_ERR)
        sws_stderror("Fail to signal child");
    return signl;
}

unsigned int sws_sleep(unsigned int second)
{
    int res_sleep;
    if ((res_sleep = sleep(second)) < 0)
        sws_stderror("Fail to sleep process");
    return res_sleep;
}

/* Re-define for memory control */

void* sws_malloc(size_t size)
{
    void *ptr;
    if ((ptr = malloc(size)) == NULL)
        sws_stderror("Fail to alloc memory");
    bzero(ptr, size);
    return ptr;
}

/* Re-define for daemonize */
void sws_daemon(int nochdir, int noclose)
{
    if (daemon(nochdir, noclose) < 0)
        sws_stderror("Fail to daemonize");
}


char* sws_getContent(char* path, int file)
{
    char* content = (char*) malloc (sizeof(char) * MAX_CONTENT_LEN);
    bzero(content, MAX_CONTENT_LEN);
    /* path indicate file */
    if (file)
    {
        int fd = open(path, O_RDONLY);
        if (fd == -1)
        {
            // open file fail
            sws_stderror("Fail to open file in sws_getContent");
            return NULL;
        }
        read(fd, content, MAX_CONTENT_LEN);

    }
    else /* path indicate directory*/
    {
        DIR* dir;
        dir = opendir(path);
        struct dirent* entry;
        while(1) {
            entry = readdir(dir);
            if (NULL == entry)
            {
                break;
            }
            if (entry->d_name[0] == '.')
                continue;
            strcat(content, entry->d_name);
            strcat(content, "\r\n");
        }
    }

    return content;
}
