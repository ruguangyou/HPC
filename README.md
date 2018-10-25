# HPC (High Performance Computing)

## Optimization
 - memory allocation: stack vs. heap
 - memory fragmentation: a big contiguous chunk vs. many small chunks but not contiguous
 - inline function
 - locality: CPU memory model
  - registers - cache L1 - cache L2 - \[cache L3\] - main memory - disks
  - cache miss
  - virtual address space
  - read by page, page fault
 - addressing: direct vs. indirect
 - data type
  - float processing tends to be more complex than int processing
  - a large array of variable with big type size would affect locality
 - casting
  - e.g. *double sqrt (double n)*, if n is int type and n = 9, call sqrt(n) would cast n to double type first
 - aliasing
  - restrict
   - e.g. double \*restrict a;
 - alignment and padding
 - disk access: HDD vs. SSD, write & read
 
## Useful tools
 - valgrind
  - tools: memcheck, cachegrind, callgrind, ...
  - e.g. valgrind --tool=memcheck ./main
 - kcachegrind
 - gprof
  - reveals how much time is spent in which function (useful for complex program)
  - requires complier flag: -pg
  - without optimization flags (i.e. -O0 or none), because too fast execution is hard to catch useful information
  - run *./main* first, then run *gprof main*
 - gcov
  - exhibits how often each code line is executed
  - requires compiler flag: -ftest-coverage -fprofile-arcs
  - without optimization flags
  - run *./main* first, then *gcov main.c*, then view the *main.c.gcov* file
 - perf
 - gdb
  - text user interface: -tui
  - input cmd line args: --args
  - for debugging multi-threads:
   - set sechduler-clocking on  // could step into one thread
   - info threads
   - thread thread_number  // switch among threads
   
## pthread

## OpenMP

## OpenCL


gdb tips:
 
