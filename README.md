# HPC (High Performance Computing)

## Notes on optimization
 - memory allocation: stack vs. heap
 - memory fragmentation: a big contiguous chunk vs. many small chunks but not contiguous
 - inline function
 - locality: CPU memory model
   - registers -- cache L1 -- cache L2 -- \[cache L3\] -- main memory -- disks
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
   


## Assignment description
http://www.math.chalmers.se/Math/Grundutb/CTH/tma881/1819/assignments.html



## pthread

#### Performance

The program is tested with the different arguments. Following tables show the results as well as the performance boundary. Times are given in seconds.

*Single thread, 1000 lines, varying polynomial x^d - 1*

| Degree of polynomial |  1   |  2   |  5   |  7   |
| :------------------: | :--: | :--: | :--: | :--: |
|      hpcuser068      | 0.03 | 0.12 | 0.27 | 0.42 |
|      hpcuser067      | 0.03 | 0.11 | 0.27 | 0.37 |
|   maximal runtime    | 1.01 | 1.48 | 1.52 | 1.64 |

*Multi threads, 1000 lines, x^5 - 1*

| Number of threads |  1   |  2   |  3   |  4   |
| :---------------: | :--: | :--: | :--: | :--: |
|    hpcuser068     | 0.27 | 0.17 | 0.12 | 0.12 |
|    hpcuser067     | 0.28 | 0.17 | 0.12 | 0.11 |
|  maximal runtime  | 1.52 | 0.70 | 0.55 | 0.42 |

*Ten threads, varying number of lines, x^7 - 1*

| Number of lines | 1000 | 50000  |
| :-------------: | :--: | :----: |
|   hpcuser068    | 0.07 | 115.30 |
|   hpcuser067    | 0.07 | 173.62 |
| maximal runtime | 0.26 |  594   |

Experimentation reveals that the limiting factors to performance are degree, number of threads and number of threads. Basically, using more threads tend to reduce the runtime. But it should be noted that there is trade-off between parallelization and communication part in terms of runtime. It should have an optimal number of threads, and if we use more threads than that, the cost of communication would impair the performance down.

When implementing the functionality, it would be good to keep in mind what kind of pattern will possibly slow the program down. For example, implementing points as work items in threads is slower than rows as work items. The former requires locking mutex quadratic times (number of points = number of lines ^ 2), far more than the latter (number of rows = number of lines). And frequently locking would block some threads that need to access the locked content, possibly causing unnecessary waiting time. 

Another thing is which function of writing is optimal. Both fprintf and fwrite are provided by stdio, but apparently they have different performance. fwrite is much faster than fprintf, as fprintf has to deal with format. When writing to output files, it would be faster to write a whole row than just write a point at a time. The latter requires quadratic fwrite calls, and calling function needs additional operations such as pushing arguments and jumping. To handle this, memcpy would a good choice to copy points data into a row buffer, and then fwrite the row.

The key to good performance relies on the implementation of compute function. Investigation by gprof tool reveals that most of the runtime is spent in compute threads. Therefore, optimization in this part would be likely to improve the performance dramatically. It makes a huge difference after applying hardcoded Newton iteration.



## OpenMP

#### Performance

Experimentation reveals that the limiting factors to performance include synchronization method, number of threads and the number of times to read from file. We did the benchmarking by implementing another C program and measuring the average runtime over 10 iterations. 

Basically, *atomic* is much faster than *critical*. The former allows access of memory atomically. The latter incurs performance loss because a thread has to enter and exit the critical section every time. Compared with *atomic*, *reduction* is even faster, since each thread has its own instance of a variable, it doesn't have to access the memory to synchronize until the end of the thread.

More threads tend to improve the performance. In our case, we got the runtime as follows (maximal cells to read at one time is 10000, compiler optimization level is O2, and runtime is measured in seconds) :

| Number of points | Number of threads | Runtime | Maximal runtime |
| :--------------: | :---------------: | :-----: | :-------------: |
|       1e4        |         1         |  0.23   |      0.41       |
|       1e5        |         5         |   4.9   |       8.2       |
|       1e5        |        10         |   2.6   |       4.1       |
|       1e5        |        20         |   1.4   |       2.6       |

As mentioned in previous section, the affect of how many cells to parse (or how many rows to read, defined as *ROW_MAX_READ*) at one time has been evaluated, with results presented in the table (1e5 points, 10 threads, runtime measured in seconds, compiler flag -O2) :

| ROW_MAX_READ | Runtime |
| :----------: | :-----: |
|     1e4      |   2.6   |
|     2e4      |   2.8   |
|    2.5e4     |   2.9   |
|     3e4      |   3.1   |
|     5e4      |   3.2   |
|     8e4      |   6.0   |
|     1e5      |   4.2   |

As is explained before, reading more cells is likely to worse the locality. The optimal number is 1e4 from our experiments.

One thing to mention is that the running results of check script may differ quite a lot in gantenbein depending on situations. So, to ensure the submission will pass the script test, we choose to use -Ofast in our final version.



## OpenCL

#### Performance

Experimentation reveals that number of boxes and number of iterations are the major factors that influence the performance. Benchmarking is done by implementing another C program that runs this program for 10 times and measures the average runtime. Results are in following table (runtime in seconds) :

|     Width*Height      | 100 * 100 | 10000 * 10000 | 100000 * 100 |
| :-------------------: | :-------: | :-----------: | :----------: |
|     Initial value     |   1e20    |     1e10      |     1e10     |
|  Diffusion constant   |   0.01    |     0.02      |     0.6      |
| Number of iterations  |  100000   |     1000      |     200      |
| Max allowable runtime |    1.7    |      98       |     1.4      |
| Runtime (hpcuser067)  |   0.78    |     34.3      |     0.49     |
| Runtime (hpcuser068)  |   0.83    |     34.1      |     0.45     |

Benchmarks surpass the expectations, and there is little difference between different users.

What we haven't done is calculating the average and average absolute difference with reduction. The program might be optimized even more or be a little overhead by using reduction. We didn't test this option, since the performance is already quite good so far.

