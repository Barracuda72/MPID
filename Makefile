SUBDIRS=config
CFLAGS=-Wall -Wextra -std=gnu11 -pedantic

all: main libLog.so

main: main.c libLog.so libmime.so libhttp.so
	@echo	Building main module...
	@gcc -g $(CFLAGS) -rdynamic -Wl,-rpath=. -o main main.c -I. -L. -lLog -ldl #-lmime

libLog.so: libLog.c libLog.h
	@echo Building libLog
	@gcc $(CFLAGS) -o libLog.so -g -fPIC -shared libLog.c -I.

libmime.so: libmime.c libmime.h
	@echo Building libMime
	@gcc $(CFLAGS) -o libmime.so -g -fPIC -shared -nostartfiles libmime.c -I.

libhttp.so:
	@make -C http

clean:
	@-rm libLog.so libmime.so libhttp.so main *~ example y.tab.c lex.yy.c *.o 2> /dev/null
	@make -C http clean
	@make -C config clean

test: libmime.so
	gcc -o test test.c -I. -L. -lmime
	LD_LIBRARY_PATH=. ./test
	rm ./test

lex:
	yacc -d text.y
	lex test.lex
	gcc lex.yy.c y.tab.c -o example

#parser:
#	yacc -d httpparser.y
#	lex httpparser.lex
#	gcc lex.yy.c y.tab.c -o example

parser: parser.c
	gcc parser.c -o parser -DDEBUG
