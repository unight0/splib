

CC = gcc
CFLAGS = -g
LD = gcc
LDFLAGS = -g

sp: main.o
	$(LD) $(LDFLAGS) $^ -o $@

main.o: main.c

clean:
	rm *.o
