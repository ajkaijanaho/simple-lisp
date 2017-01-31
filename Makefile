
CC = gcc --std=c99
CFLAGS = -Wall -Wextra -g 
LDFLAGS = 
LDLIBS = -lreadline -lgc

OBJ = 	simple-lisp.o ast.o data.o error.o eval.o lexer.o \
	primops.o printer.o strvec.o y.tab.o

simple-lisp : $(OBJ)
	$(CC) $(LDFLAGS) -o $@ $^ $(LDLIBS)

y.tab.c y.tab.h : sexp.y
	$(YACC) -d -v $<

clean :
	$(RM) simple-lisp $(OBJ) y.tab.c y.tab.h

ast.o: ast.c ast.h data.h config.h error.h strvec.h
data.o: data.c data.h config.h error.h
error.o: error.c error.h config.h
eval.o: eval.c ast.h data.h config.h error.h eval.h primops.h
lexer.o: lexer.c data.h config.h y.tab.h lexer.h
primops.o: primops.c primops.h data.h config.h
printer.o: printer.c error.h config.h printer.h data.h
simple-lisp.o: simple-lisp.c
strvec.o: strvec.c error.h config.h strvec.h
y.tab.o: y.tab.c data.h config.h eval.h lexer.h printer.h
