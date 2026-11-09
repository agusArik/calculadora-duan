CC = gcc
CFLAGS = -std=c11 -Wall -Wextra -g -D_DEFAULT_SOURCE
OBJS = calculadora.o lexer.o ast.o parser.o evaluador.o

calculadora: $(OBJS)
	$(CC) $(CFLAGS) -o $@ $(OBJS) -lm

%.o: %.c lexer.h ast.h parser.h evaluador.h
	$(CC) $(CFLAGS) -c $<

clean:
	rm -f calculadora *.o

.PHONY: clean
