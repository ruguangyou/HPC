# Assignment report on openmp (hpcgp033)

## Program layout

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

## Performance

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

