#	libHTTP

CFLAGS= -I../ -I./ -L../ -Wall -Wextra -std=gnu11 -pedantic
CC=fakecc

all:	libhttp.so

libhttp.so: parser.c parser.h libhttp.c http_ans.h config
	@echo Building libHTTP...
	@gcc $(CFLAGS) -c libhttp.c -o libhttp.o -fPIC
	@gcc $(CFLAGS) -c parser.c -o parser.o -fPIC
	@gcc $(CFLAGS) -o libhttp.so -g parser.c libhttp.c htconfig.l.c htconfig.y.c -shared -lmime -fPIC
	@cp libhttp.so .. 

config: htconfig.l htconfig.y confclean
	@lex -o htconfig.l.c htconfig.l 
	@yacc -d htconfig.y -o htconfig.y.c
#	@gcc -c htconfig.l.c htconfig.y.c
#	@gcc htconfig.l.o htconfig.y.o -o configread

conftest: htconfig.l htconfig.y confclean
	@lex -o htconfig.l.c htconfig.l 
	@yacc -d htconfig.y -o htconfig.y.c
	@gcc -g -c htconfig.l.c htconfig.y.c -D__CONFTEST__
	@gcc htconfig.l.o htconfig.y.o -o configread

confclean:
	@-rm htconfig.l.c htconfig.y.c

clean: confclean
	@-rm *.o *~ htconfig.l.c htconfig.y.c libhttp.so
