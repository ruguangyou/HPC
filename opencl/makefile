LIB=/usr/local/cuda-10.0/targets/x86_64-linux/lib

.PHONY : all clean

all : heat_diffusion

heat_diffusion : heat_diffusion.c
	gcc -o $@ $^ -std=c99 -L$(LIB) -lOpenCL

benchmark : benchmark.c time_diff.c
	gcc -o $@ $^

runtime :
	./benchmark ./heat_diffusion 100 100 -i1e20 -d0.01 -n100000
	#./benchmark ./heat_diffusion 10000 10000 -i1e10 -d0.02 -n1000 
	#./benchmark ./heat_diffusion 100000 100 -i1e10 -d0.6 -n200

debug : heat_diffusion.c
	gcc -o $@ $^ -std=c99 -L$(LIB) -lOpenCL -g 

db :
	gdb -tui --args ./debug 1000 10 -i1e3 -d0.01 -n2
	#./debug 3 3 -i1e6 -d0.0333333333333 -n2

tar :
	tar czf heat_diffusion.tar.gz heat_diffusion.c heat_diffusion_kernel.cl makefile benchmark.c time_diff.c time_diff.h report.md

clean :
	rm -f heat_diffusion debug benchmark