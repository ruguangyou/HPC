# Assignment report on threads (hpcgp033)

## Program layout

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

~~~ c
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
~~~

After one iteration, it needs to check the results. First compare the absolute value of real and imaginary part to an upper bound. Without abs function, this is done by comparing with 0 and taking positive value. Then compare the distance to origin to a lower bound. To avoid computing square root, the lower bound is squared. Similarly, the squared distance to one of the pre-computed roots is compared to the squared lower bound as well. When the iteration of one point is finished, use memcpy to write corresponding color and gray string into local *attractor* and *convergence*. When a row is finished, change the default value 0 in *row_done* to 1, and here the writing should be protected by mutex, as in the write thread *row_done* will be read to judge whether to write or not.

In write thread, also work row by row. Use sprintf to set file name and then open the files. Judge if a row has done. if so write the *attractor* and *convergence* to corresponding files. If not sleep 100 micro seconds and wait the results from compute threads.

When compiling O2 optimization flag is applied.

## Performance

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

## Further questions

Regarding on the further exploration mentioned on the assignment instruction, here is some guessing without actual experimenting. 

- If dropping the assumption that the degree is small, then we could still hardcode some simple cases, but for large degree hardcode is impractical, so maybe just use a for loop to compute x^(d-1) and then x - 1/d - 1/(d*x^(d-1))
- For very large numbers of lines, there might be some issues about memory to consider. In my own laptop, I got segmentation fault (core dumped) error when I tried to run the case wit 50000 lines, and what valgrind memcheck told was "Access not within mapped region at address 0x0". So definitely a more sophisticated and flexible partitioning of the work is needed but we have no idea about how to do now.
- Current distance checking has complexity O(d). But if we can sort out the roots, it's possible to use binary search. However, could not build a clear picture in mind about how to combine Taylor expansion and binary search.