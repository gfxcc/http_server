CC = cc
CFLAGS = -pipe -O0 -W -Wall -Wpointer-arith -Wno-unused-parameter -Wno-unused-function -Werror -g
LINK = $(CC)

DEPS = filelog.h \
	   http.h \
	   server.h \
	   sws.h \
	   sws_define.h

INCS = -I ./

OBJS = filelog.o \
	   http.o \
	   server.o \
	   sws_define.o

default: build

build: sws

clean:
	rm -rf *.o
	rm -rf sws


sws: sws.o $(OBJS)
	$(LINK) -o sws sws.o $(OBJS)

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

