CC=gcc
LIB=-lm -lpthread
FLAG=-march=native -O2 

newton : newton.o
	$(CC) -o $@ $^ $(LIB) $(FLAG)

newton.0 : newton.c
	$(CC) -o $@ -c $< $(LIB) $(FLAG)


.PHONY : clean

clean :
	rm newton newton.o
