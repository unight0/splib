

CC = gcc
CFLAGS = -g
LD = gcc
LDFLAGS = -g

sp: main.o lexer.o parser.o
	$(LD) $(LDFLAGS) $^ -o $@

main.o: main.c

lexer.o: lexer.c

parser.o: parser.c

clean:
	rm *.o
