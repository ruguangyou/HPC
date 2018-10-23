CC=gcc
LIB=-lm -pthread
FLAG=-O2 -march=native

.PHONY : all clean

all : newton newton.o complex.o

newton : newton.o complex.o
	$(CC) -o $@ $^ $(LIB) $(FLAG)

newton.o : newton.c
	$(CC) -o $@ $^ -c $(LIB) $(FLAG)

complex.o : complex.c
	$(CC) -o $@ $^ -c $(FLAG)

clean :
	rm -f newton *.o *.ppm

