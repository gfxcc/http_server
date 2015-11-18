//
//  server.h
//  sws
//
//  Created by Chen Wei on 11/14/15.
//  Copyright © 2015 Chen Wei. All rights reserved.
//

#ifndef server_h
#define server_h

#include "sws_define.h"

typedef struct opts_props
{
    int debug_mode;     /* -d option, allow debug mode */
    char *cgi_dir;      /* -c option, allow exec of CGIs from the given dir */
    char *ip_address;   /* -i option, bind with the given ip_address */
    char *file_log;     /* -l option, log all request to the given file */
    char *port;         /* -p option, listen to the given port, default:8080 */
}st_opts_props;

void sig_chld_handler(int sig);
void init_opts_props(st_opts_props *sop);
void server_exec(st_opts_props *sop);

#endif /* server_h */
