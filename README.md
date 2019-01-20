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
   



http://www.math.chalmers.se/Math/Grundutb/CTH/tma881/1819/assignments.html

## pthread

#### Program layout

The program is implemented as several functions in several files. The files include:

- newton.c, containing main function, and functions for threads,
- complex.c and complex.h, implementing calculation of complex number in one iteration of Newton's method.

The required include files are standard headers, math and pthread library headers, as well as a user-defined header:

- output of the error messages and strings needed by PPM files (fprintf, sprintf, fopen and fwrite provided by stdio),
- conversion of input command from string to integer (strtol provided by stdlib),
- termination of process if errors were raised (exit provided by stdlib),
- heap memory management (malloc, calloc and free provided by stdlib),
- copying of strings (memcpy provided by string),
- calculation of cos and sin values (cos and sin provided by math),
- threads creation and synchronization (pthread_create, pthread_join, pthread_mutex_lock and pthread_mutex_unlock provided by pthread),
- a source of routine sleeping (nanosleep provided by time),
- calculation of Newton's method (complex_dn provided by complex).

Before the main function, some global variables are declared, which are shared by threads.

The layout of the main function consists of:

- parsing the command line arguments (number of lines, number of threads, degree), and initializing the corresponding global variables,
- allocating memory for the global pointers and initializing:
  - pre-computing roots of f(x) = x^d - 1,
  - discretization of the complex plane (range from -2 to 2 for both real and imaginary part),
  - pre-set color strings and gray strings that will be used when writing attractor and convergence files,
  - deciding which hardcoded function should be used given the degree.
- creating compute threads and write threads, and joining them before terminating,
- finalizing (destroying mutex, freeing allocated memory).

The layout of newton (compute) function consists of:

- gaining offset from the passed argument, and initializing local variables,
- dividing workload row by row for the threads,
- in each row, computing the points from left to right,
- in each point calculation, iterating till one of the conditions (the absolute value hits upper boundary, too close to the origin, or converge to a root) is met, and recording the root and number of iterations,
- locking and writing the "row done" status,
- finalization.

The layout of write_to_disc (write) function consists of:

- initialization,
- opening files with specific names,
- checking whether a row has been done, if not sleeping for a while (100 micro seconds),
- if done, retrieving the long strings for the row and write to  the corresponding files,
- finalization (freeing memory, closing files).

Simplifications include:

- assuming the degree is small (less than 10), and hardcoding optimal formulas case by case,
- assuming the number of lines if not too large (less than 100000), and thus no need to worry about allocating a very large memory block.

Roots are pre-computed via theoretical formulas. 

Discretization of real and imaginary part ranging from -2 to 2 is determined by how many lines is wanted. The interval is calculated via 4 / (num_lines - 1). And when storing these initial values in array, the indexes of real and imaginary array are different.

The color string is set to taking six chars, e.g. "1 3 5 " (three digits and three spaces). As it is assumed degree less than 10, the combinations of three single digits should be enough to represent colors for roots. The gray string is set to taking 12 chars, e.g. "058 058 058 " (9 digits and 3 spaces). The maximum number of iterations is predefined, and the pre-setting is just using sprintf writing numbers from 0 to max_iteration to gray string.

Given the assumption that degree and maximum number of iterations are small, the char type is used to encode the attractor and convergence index associated with a pixel. The arrays *attractors* and *convergences* are global and keep track of the local *attractor* and *convergence*. The local arrays are allocated in compute threads and freed in write thread.

In the switch..case.. block, a function pointer is set to the function of hardcoded formula with the given degree.

 Then in for the loop, a specific number of threads are created for computing, and an additional thread is created for writing. The mechanism of join and mutex ensures the synchronization.

In the compute threads, picture is computed row by row. To accelerate the running speed, the Newton iteration formula x - (x^d - 1)/(d * x^(d-1)) is simplified x - x/d - 1/(d*x^(d-1)). And by making the assumption that degree less than 10, one can hardcode the optimal formula, for example (d = 7):

```c
void complex_d7 (double *cpx) {
    double a, b, a2, b2, a4, b4, a6, b6, ab6;
    a = cpx[0];
    b = cpx[1];
    a2 = a*a-b*b;
    b2 = 2*a*b;
    a4 = a2*a2-b2*b2;
    b4 = 2*a2*b2;
    a6 = a2*a4-b2*b4;
    b6 = a2*b4+b2*a4;
    ab6 = a6*a6+b6*b6;
    cpx[0] = (6*a+a6/ab6)/7;
    cpx[1] = (6*b-b6/ab6)/7;
}
```

After one iteration, it needs to check the results. First compare the absolute value of real and imaginary part to an upper bound. Without abs function, this is done by comparing with 0 and taking positive value. Then compare the distance to origin to a lower bound. To avoid computing square root, the lower bound is squared. Similarly, the squared distance to one of the pre-computed roots is compared to the squared lower bound as well. When the iteration of one point is finished, use memcpy to write corresponding color and gray string into local *attractor* and *convergence*. When a row is finished, change the default value 0 in *row_done* to 1, and here the writing should be protected by mutex, as in the write thread *row_done* will be read to judge whether to write or not.

In write thread, also work row by row. Use sprintf to set file name and then open the files. Judge if a row has done. if so write the *attractor* and *convergence* to corresponding files. If not sleep 100 micro seconds and wait the results from compute threads.

When compiling O2 optimization flag is applied.

#### Performance

The program is tested with the different arguments. Following tables show the results as well as the performance boundary. Times are given in seconds. Some benchmarks from the two hpc users have big differences, however, we could not find a theoretical explanation for this.

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

#### Program layout

The program is implemented as a single main function in one file. The required include files are standard headers, math library header and openmp library header:

- input from file, and navigating inside the file (fopen, fread, fseek, ftell, fclose provided by stdio)
- output to stdout (fprintf provided by stdio)
- conversion of command line argument from string to integer (strtol provided by stdlib)
- calculation of square root (sqrt provided by math)
- implementation of parallelism (API provided by omp, such as #pragma parallel for)

The coarse layout of the main function consists of five parts:

- parsing command line argument and set the number of threads,
- declaration of variables and stack arrays,
- parsing the file and reading cell coordinates from it,
- computing cell distances in parallel, and incrementing distance counts accordingly,
- printing out a sorted list of distance counts to stdout.

OpenMP parallelism is applied in this program. Number of threads is given via command line arguments. The coordinates in the file *cells* have a fixed format; each row corresponds to three coordinates of one cell, following the pattern: a sing '+' or '-', two digits, a decimal point '.', three digits, and a blank space ' ' or newline character '\n'. So there are 24 characters in each row. This fixed input format provides convenience for file parsing. Coordinates reading is carried out in a double for-loop: the outer loop reads the row, and the inner loop reads three coordinates corresponding to the row. *#pragma omp parallel for* divides up the reading task into multi-threads. Since the outer and inner loop has no data dependency, *collpase(2)* clause is added to parallelize further. When converting the strings to the corresponding numerical values, a feasible way in C is to subtract '0' from the digit character. 

After parsing, the coordinates are stored in short type array. With the simplification that coordinates are between -10 and 10, as well as the fixed input format (example of conversion: '+09.045' to 9045), it's not necessarily to use floating point data types. As the min and max value after conversion is -10000 and 10000, the short type (2 bytes, \[-32768,32767\]) would be a better candidate than float and double type which take 4 bytes and 8 bytes respectively. CPU processing would benefit from the smaller data type, with which more coordinates can be loaded into cache and help improving locality. Similarly, the output format is also fixed: two digits, a decimal point '.', and two digits. The distance results range from 00.00 to 34.65 (the max distance (20^2 + 20^2 + 20^)^0.5 ), so when recording the distance counts, an efficient way is to keep a size_t type array with length 3466, the array indexes 0, 1, ..., 3465 represent the cell distances. When computing square roots, however, the short type coordinates has to be converted to double type, as the signature of sqrt function is *double sqrt (double n)*. This converting would incur performance loss of about 76%, by benchmarking a program that computes the sum of the first 1e9 integers with (5.1 s runtime) and without (2.9 s runtime) cast statement.

The program at no time may consume more than 1 GB of memory. When parsing the cells, it's possible to exceed the memory limit if reading all the cells into one memory chunk. The only assumption of the number of cells is that there is less than 2^32, and we still risk at exceeding limit when there is, for example 2^10 cells, unless we read the file in separate times. However, file access is relatively time consuming, so it's good to avoid reading from file frequently. So there is a trade off between reading more times or less times, the former would cause high latency, and the latter would lead to big memory chunk together with bad memory locality. The read size is set to 10000 lines at each time. It is obtained by experiments presented in *Performance* section. 10000 rows, each row with 24 bytes, and converting to 3*10000 coordinates, each coordinate with 2 bytes (short type), the consumed memory is 10000\*24+3\*10000\*2=300000 bytes. So it's not likely to use more than 1 GB memory even taking into account some local variables and buffer arrays.

Parallelization enters at several points of the program. *#pragma omp parallel for* is used in parsing coordinates and counting distances. The way openmp works is creating a team of threads and allocating computing tasks into different threads. In the program a macro-defined constant *ROW_MAX_READ* tells how many cells to read each time at most. In case of very large number of cells, parsing and counting are divided up into several blocks. The program uses a for-loop to read block *i (i = 0, 1, ..., n_blocks-1)*, and counts cell distances inside this block. Then it reads another block *j (j = i+1, i+2, ..., n_blocks-1)*, and counts cell distance between block i and j. There is no synchronization issue with parsing since the program doesn't read two blocks simultaneously, and conversion of cell coordinates are independent from each other. However, there is synchronization issue with counting, as different threads will write distance counts at the same time. The way to synchronize is to use *#pragma omp critical* or *#pragma omp atomic*, but it turns out that using *reduction* would be optimal.

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

#### Program layout

The program is implemented as a main function in a .c file as well as a kernel function in a .cl file, which uses the GPU via OpenCL. The required include files are standard headers and OpenCL library header:

- inputting OpenCL kernel function source code from .cl file as C strings (fopen, fread, fseek, ftell and fclose provided by stdio)
- outputting error messages or computing results (printf provided by stdio)
- converting string to numerical value (strtol, strtod provided by stdlib)
- memory management (malloc, calloc, free provided by stdlib)
- using GPU via OpenCL APIs (clGetPlatformIDs, clGetDeviceIDs, etc. provided by cl)

The coarse layout of the main function consists of:

- parsing command line arguments
- variables and arrays initialization
- reading kernel function code and storing it in a C string
- setup for OpenCL, including:
  - get platform, get device
  - create context, create command queue
  - create program with source, build program
  - create buffer, write buffer to command queue
  - create kernel, set kernel arguments
  - execute kernel
  - read buffer from command queue
- computing average and absolute average difference, and printing them out
- finalization (release OpenCL objects, free allocated memory)

A simplified model for heat diffusion in 2-D space is split up into small boxes, with size width and height given by the command line arguments. The initialized value of temperature is set to zero, except for the middle point(s) whose initial value is determined by a parameter *-i*. The initial value might to divided up between 2 or 4 central points depending on whether the width or height is even. 

OpenCL parallelism is used in this program. APIs provided by OpenCL provide a way to employ GPU to compute each box's heat diffusion in parallel. The OpenCL platform consists of a host (e.g. CPU) and some devices (e.g. GPU, FPGA), and a context is created with device(s) to manage objects such as command queue, program and kernel. The command queue contains instructions of memory written and read as well as execution of kernel in some order. And the kernel is executed in work-items of the device. After setting up platform, device, context, command queue, program and kernel, GPU is ready to take inputs and execute kernel function. A buffer on the device memory is then created and copies values from host memory via command queue, since device memory and host memory are separated in OpenCL memory model. After GPU finishes processing, the buffer is read back again to host memory.

The kernel execution uses 2-D range, with width and height of the boxes as x and y dimension in respective. The boxes is padded for the convenience of computing. Temperature of the boxes in the pads will always be 0. For the inside boxes, this setting make sure that taking values from left, right, up, down side boxes won't cause segmentation fault. After getting new value of a box, there is a barrier*(CLK_GLOBAL_MEM_FENCE)*  so that updating of the box won't be carried out until other boxes are ready for updating. The iteration loop is implemented in host side to make better synchronization, i.e. all boxes in buffer are updated after each iteration. (One question related to this is: implement iteration loop in the kernel would cause race condition even though barrier is set. Haven't found reasonable explanation so far.)

After computation is done on GPU, the updated values of each box is copied back to host memory. The following two for-loops are summing the values and absolute differences respectively, and then they are divided by the number of boxes to get the average. Two things should be noted: the type of variable that accumulates box's values should be double such that overflow could be avoided; padding should be taken into account as boxes in pads contribute to absolute differences.

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

