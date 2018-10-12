CC=gcc
LIB=-lm -pthread
FLAG=-O2 

benchmark : benchmark.o time_diff.o complex.o
	$(CC) -o $@ $^ $(LIB) $(FLAG)

benchmark.o : newton.c #benchmark.c
	$(CC) -o $@ -c $< $(LIB) $(FLAG)

time_diff.o : time_diff.c
	$(CC) -o $@ -c $^ $(FLAG)

complex.o : complex.c
	$(CC) -o $@ -c $^ $(FLAG)

.PHONY : clean

clean :
	rm -f benchmark *.o *.ppm
