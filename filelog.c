//
//  filelog.c
//  sws
//
//  Created by Chen Wei on 11/15/15.
//  Copyright © 2015 Chen Wei. All rights reserved.
//

#include "http.h"
#include "server.h"
#include "filelog.h"
#include "sws_define.h"

int filelog_init(st_opts_props *sop)
{
    if (sop->file_log != NULL)
    {
        int fd_log;
        while ((fd_log = sws_open(sop->file_log, O_APPEND | O_CREAT | O_WRONLY | S_IRUSR
                                 | S_IWUSR | S_IRGRP, S_IRUSR | S_IWUSR | S_IRGRP)) < 0);
        return fd_log;
    }
    else return -1;
}

void filelog_record(st_opts_props *sop, st_log *log)
{
    int fd_log = filelog_init(sop);
    if(fd_log != -1)
    {
        flock(fd_log, LOCK_EX);
        char logline[MAX_BUFFER_LEN];
        sprintf(logline, "%s: %s\t%s\t%d\t%lu\n", log->ip_addr, log->time, log->req, log->http_status, log->resp_len);
        sws_write(fd_log, logline, MAX_BUFFER_LEN); close(fd_log);
        flock(fd_log, LOCK_UN);
        
        if (sop->debug_mode)
            printf("%s\n", logline);
    }
    else return;
}