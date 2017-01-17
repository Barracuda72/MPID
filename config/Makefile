all: config.o

config.o: config.c
	gcc -c config.lex.c config.y.c

config.c: config.lex.c config.y.c

config.lex.c:
	lex -o config.lex.c config.lex

config.y.c:
	yacc -d config.y -o config.y.c

clean:
	@-rm *.c *.h *.o 2> /dev/null
