

CC = gcc
CFLAGS = -g
LD = gcc
LDFLAGS = -g

#CC = afl-gcc-fast
#LD = afl-gcc-fast

sp: main.o
	$(LD) $(LDFLAGS) $^ -o $@

main.o: main.c

clean:
	rm *.o
