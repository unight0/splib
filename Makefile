
CC = gcc
CFLAGS = -g -I.

sp: examples/main.c splib.h
	$(CC) $(CFLAGS) examples/main.c -o $@

fuzz: examples/main.c splib.h
	AFL_CC=gcc afl-gcc-fast $(CFLAGS) examples/main.c -o sp-fuzz

fuzz-run: fuzz
	afl-fuzz -i in-fuzz -o findings-fuzz ./sp-fuzz

clean:
	rm -f sp sp-fuzz
