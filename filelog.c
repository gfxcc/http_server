//
//  filelog.c
//  sws
//
//  Created by Chen Wei on 11/15/15.
//  Copyright (C) 2015 Chen Wei. All rights reserved.
//

#include "sws.h"
#include "http.h"
#include "server.h"
#include "filelog.h"
#include "sws_define.h"

int filelog_init(st_opts_props *sop, char* erro)
{
    if (sop->file_log != NULL)
    {
        char logfile[PATH_MAX];
        realpath(sop->file_log, logfile);
        //strcpy(logfile, erro);
        //strcat(logfile, sop->file_log);
        int fd_log;
        fd_log = open(logfile, O_APPEND | O_CREAT | O_WRONLY);
        if (fd_log == -1)
        {
            sws_stderror("open log file fail");
        }
        return fd_log;
    }
    else return -1;
}

void filelog_record(st_opts_props *sop, st_log *log, char* erro)
{
    int fd_log = filelog_init(sop, erro);
    if(fd_log != -1)
    {
        /* flock(fd_log, LOCK_EX); */
        char logline[MAX_BUFFER_LEN]; bzero(logline, MAX_BUFFER_LEN);

        sprintf(logline, "%s %s \"%s\" %d %lu\n\n",
                log->ip_addr, log->time, log->req, log->http_status, log->resp_len);

        while(write(fd_log, logline, strlen(logline)) < 0);

        /* flock(fd_log, LOCK_UN); */
        close(fd_log);
        if (sop->debug_mode) {
            printf("%s\n", logline);
        }
    }
    else return;
}
