
//#include <magic.h>
//#include <string.h>
#include <errno.h>

#include "sws.h"
#include "server.h"
int init_magic(  st_opts_props*server_props ){
    server_props->cookie = magic_open(MAGIC_MIME_TYPE);
    if (server_props->cookie == NULL) {
        fprintf(stderr,"initialize magic error: %s\n",strerror(errno));
        return 1;
    }

    if (magic_load(server_props->cookie, NULL) != 0) {
        fprintf(stderr,"cannot load magic database : %s\n", magic_error(server_props->cookie));
        magic_close(server_props->cookie);
        return 1;
    }
    return 0;

}

const char* get_magictype(st_opts_props*server_props , char*path){
    const char*tmp = magic_file(server_props->cookie,path);
    if(tmp==NULL)
         return "text/html\r\n";
    else 
         strcat((char*)tmp,"\r\n");
         return (const char*)tmp;


}
