CC=gcc
LIB=-lm -pthread
FLAG=-march=native -O2 

benchmark : benchmark.o time_diff.o
	$(CC) -o $@ $^ $(LIB) $(FLAG)

benchmark.o : benchmark.c
	$(CC) -o $@ -c $< $(LIB) $(FLAG)

time_diff.o : time_diff.c
	$(CC) -o $@ -c $^ $(FLAG)

.PHONY : clean

clean :
	rm benchmark *.o
