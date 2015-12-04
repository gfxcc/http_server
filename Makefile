CC = cc
CFLAGS = -std=gnu99 -pipe -O0 -W -Wall -Wpointer-arith -Wno-unused-parameter -Wno-unused-function -Werror -g
LINK = $(CC)

DEPS = filelog.h \
	   http.h \
	   server.h \
       magic_type.h \
	   sws.h \
	   sws_define.h \
	   cgi.h

INCS = -I ./

OBJS = filelog.o \
	   http.o \
	   server.o \
	   sws_define.o \
	   cgi.o \
       magic_type.o

default: build

build: sws

clean:
	rm -rf *.o
	rm -rf sws


sws: sws.o $(OBJS)
	$(LINK) -o sws sws.o $(OBJS) -lmagic

sws.o: sws.c $(DEPS)
	$(CC) -c $(CFLAGS) $(INCS) -o sws.o sws.c

filelog.o: filelog.c $(DEPS)
	$(CC) -c $(CFLAGS) $(INCS) -o filelog.o filelog.c

http.o: http.c $(DEPS)
	$(CC) -c $(CFLAGS) $(INCS) -o http.o http.c

server.o: server.c $(DEPS)
	$(CC) -c $(CFLAGS) $(INCS) -o server.o server.c

sws_define.o: sws_define.c $(DEPS)
	$(CC) -c $(CFLAGS) $(INCS) -o sws_define.o sws_define.c

cgi.o: cgi.c $(DEPS)
	$(CC) -c $(CFLAGS) $(INCS) -o cgi.o cgi.c

magic_type.o: magic_type.c $(DEPS)
	$(CC) -c $(CFLAGS) $(INCS) -o magic_type.o magic_type.c
