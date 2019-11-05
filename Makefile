CC=gcc
CFLAGS=-I
OUT=/out
DEPS

main: main.o
	CC $OUT/main.o -o $OUT/main

main.o: main.c
	CC -c main.c -o $OUT/main.o

clean:
	ERASE $OUT
