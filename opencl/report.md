# Assignment report on OpenCL (hpcgp033)

## Program layout

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

## Performance

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