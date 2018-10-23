.PHONY : all clean runtime

all : cell_distance 

cell_distance : cell_distance.o
	#gcc -o $@ $^ -std=c99 -O2 -lm -lgomp
	gcc -o $@ $^ -std=c99 -Ofast -lm -lgomp

cell_distance.o : cell_distance.c
	#gcc -c $^ -std=c99 -O2 -lm -fopenmp
	gcc -c $^ -std=c99 -Ofast -lm -fopenmp

benchmark : benchmark.c time_diff.c
	gcc -o $@ $^

debug : cell_distance.c
	gcc -o $@ $^ -std=c99 -g -lm -lgomp

clean :
	rm -f *.o debug timeit benchmark cell_distance

runtime :
	#./benchmark ./cell_distance -t1
	./benchmark ./cell_distance -t5
	#./benchmark ./cell_distance -t10
	#./benchmark ./cell_distance -t20

tar :
	rm -rf ../a3_grading/distances ../a3_grading/extracted ../a3_grading/reports
	tar czf cell_distance.tar.gz makefile cell_distance.c report.md
	cp cell_distance.tar.gz ../a3_grading/

