//
//  filelog.h
//  sws
//
//  Created by Chen Wei on 11/15/15.
//  Copyright © 2015 Chen Wei. All rights reserved.
//

#ifndef filelog_h
#define filelog_h

#include "http.h"
#include "server.h"
#include "sws_define.h"

int filelog_init(st_opts_props *sop);
void filelog_record(st_opts_props *sop, st_log *log);

#endif /* filelog_h */
