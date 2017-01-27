
CC = gcc --std=c99
CFLAGS = -Wall -Wextra -g 
LDFLAGS = 
LDLIBS = -lreadline -lgc

OBJ = simple-lisp.o data.o error.o lexer.o printer.o y.tab.o

simple-lisp : $(OBJ)
	$(CC) $(LDFLAGS) -o $@ $^ $(LDLIBS)

y.tab.c y.tab.h : sexp.y
	$(YACC) -d -v $<

clean :
	$(RM) simple-lisp $(OBJ) y.tab.c y.tab.h

data.o : data.c data.h error.h
error.o : error.c error.h
lexer.o : lexer.c lexer.h y.tab.h
printer.o : printer.c printer.h data.h
y.tab.o : y.tab.c y.tab.h data.h lexer.h printer.h


