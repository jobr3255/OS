CC = gcc
CFLAGS = -pthread -Wall -Wextra

.PHONY: all clean

all: multi-lookup

multi-lookup: multi-lookup.c util.c FileBuffer.c ArrayBuffer.c
	$(CC) $(CFLAGS) -o multi-lookup multi-lookup.c

util.o: util.c util.h
	$(CC) $(CFLAGS) $<

clean:
	rm -f multi-lookup
	rm -f *.o
	rm -f *~
