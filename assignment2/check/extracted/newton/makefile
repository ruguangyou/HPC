CC=gcc
LIB=-lm -pthread
FLAG=-O2 # -g # -pg -ftest-coverage -fprofile-arcs

newton : newton.o complex.o
	$(CC) -o $@ $^ $(LIB) $(FLAG)

newton.o : newton.c
	$(CC) -o $@ $^ -c $(LIB) $(FLAG)

complex.o : complex.c
	$(CC) -o $@ $^ -c $(FLAG)

.PHONY : clean

clean :
	rm -f newton *.o *.ppm

.PHONY : tar

tar :
	tar czf newton.tar.gz newton.c complex.c complex.h makefile
	rm -f check/newton.tar.gz
	rm -rf check/extracted/ check/pictures/ check/reports/
	mv newton.tar.gz check/
