#define CL_USE_DEPRECATED_OPENCL_1_2_APIS
#include <CL/cl.h> 
#include <stdio.h> 
#include <stdlib.h> 

int main (int argc, char **argv) {
    if (argc != 6) {
        printf("Usage example: ./heat_diffusion 1000 100 -i1e10 -d0.02 -n20\n");
        return 1;
    }
    // parse command line arguments
    char *endptr;
    size_t width, height, iters, nmb_boxes, nmb_boxes_padding;
    float init_value, diffusion_const;
    width = 0; height = 0;
    for (int i = 1; i < argc; ++i) {
        // strtol: string to long int
        // strtod: string to double
        if (argv[i][0] != '-') {
            if (width == 0)
                width = strtol(argv[i], &endptr, 10);
            else 
                height = strtol(argv[i], &endptr, 10);
        }
        else if (argv[i][1] == 'i') {
            init_value = strtod(argv[i]+2, &endptr);
        }
        else if (argv[i][1] == 'd') {
            diffusion_const = strtod(argv[i]+2, &endptr);
        }
        else if (argv[i][1] == 'n') {
            iters = strtol(argv[i]+2, &endptr, 10);
        }
        else {
            printf("unexpected argument\n");
            return 1;
        }
    }

    // initialize
    double avg, avg_abs_diff;
    nmb_boxes = width*height;
    // add two padding on each side
    width += 2; height += 2;
    nmb_boxes_padding = width*height;
    float *boxes = (float *) calloc(nmb_boxes_padding, sizeof(float));
    int width_odd = width % 2;
    int height_odd =height % 2;
    size_t mid_width = width / 2;
    size_t mid_height = height / 2;
    if (width_odd && height_odd) {
        // both are odd numbers
        boxes[mid_height*width + mid_width] = init_value;
    }
    else if (width_odd && !height_odd) {
        // width is odd, height is even
        init_value /= 2;
        boxes[(mid_height-1)*width + mid_width] = init_value;
        boxes[mid_height*width + mid_width] = init_value;
    }
    else if (!width_odd && height_odd) {
        // width is even, height is odd
        init_value /= 2;
        boxes[mid_height*width + mid_width - 1] = init_value;
        boxes[mid_height*width + mid_width] = init_value;
    }
    else {
        // both are even numbers
        init_value /= 4;
        boxes[(mid_height-1)*width + mid_width - 1] = init_value;
        boxes[(mid_height-1)*width + mid_width] = init_value;
        boxes[mid_height*width + mid_width - 1] = init_value;
        boxes[mid_height*width + mid_width] = init_value;
    }

    // read kernel source code into c string
    FILE *fp = fopen("heat_diffusion_kernel.cl", "r");
    if (!fp) {
        printf("failed to load kernel\n");
        return 1;
    }
    size_t src_size;
    char *opencl_program_src;
    fseek(fp, 0, SEEK_END);
    src_size = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    opencl_program_src = (char *) malloc(src_size * sizeof(char));
    if (opencl_program_src == NULL) {
        printf("could not allocate memory\n");
        return 1;
    }
    src_size = fread(opencl_program_src, sizeof(char), src_size, fp);
    fclose(fp);

    cl_int error, status;
    // get platform information
    cl_platform_id platform_id;
    cl_uint nmb_platforms;
    if (clGetPlatformIDs(1, &platform_id, &nmb_platforms) != CL_SUCCESS) {
        printf("cannot get platform\n");
        return 1;
    }

    // get device information
    cl_device_id device_id;
    cl_uint nmb_devices;
    if (clGetDeviceIDs(platform_id, CL_DEVICE_TYPE_GPU,
          1, &device_id, &nmb_devices) != CL_SUCCESS) {
        printf("cannot get device\n");
        return 1;
    }

    // create an opencl context
    cl_context context;
    cl_context_properties properties[] = { CL_CONTEXT_PLATFORM,
          (cl_context_properties) platform_id, 0 };
    context = clCreateContext(properties, 1, &device_id, NULL, NULL, &error);
    if (error != CL_SUCCESS) {
        printf("cannot create context\n");
        return 1;
    }

    // create a command queue
    cl_command_queue command_queue;
    command_queue = clCreateCommandQueue(context, device_id, 0, &error);
    if (error != CL_SUCCESS) {
        printf("cannot create commamd queue\n");
        return 1;
    }

    // create a program from kernel source code
    cl_program program;
    program = clCreateProgramWithSource(context, 1, 
          (const char **) &opencl_program_src, 
          (const size_t *) &src_size, &error);
    if (error != CL_SUCCESS) {
        printf("cannot create program object\n");
        return 1;
    }

    // build the program
    // error = clBuildProgram(program, 1, &device_id, NULL, NULL, NULL);
    error = clBuildProgram(program, 0, NULL, NULL, NULL, NULL);
    if (error != CL_SUCCESS) {
        printf("cannot build program. log:\n");
        size_t log_size = 0;
        clGetProgramBuildInfo(program, device_id, CL_PROGRAM_BUILD_LOG,
              0, NULL, &log_size);
        char *log = calloc(log_size, sizeof(char));
        if (log == NULL) {
            printf("could not allocate memory\n");
            return 1;
        }
        clGetProgramBuildInfo(program, device_id, CL_PROGRAM_BUILD_LOG,
            log_size, log, NULL);
        printf("%s\n", log);
        free(log);
        return 1;
    }

    // create opencl kernel
    cl_kernel kernel;
    kernel = clCreateKernel(program, "diffusion", &error);
    if (error != CL_SUCCESS) {
        printf("cannot create kernel\n");
        return 1;
    }

    // create memory buffers on the device (i.e. GPU)
    cl_mem buffer_boxes = clCreateBuffer(context, CL_MEM_READ_WRITE,
          sizeof(float)*nmb_boxes_padding, NULL, &error);
    if (error != CL_SUCCESS) {
        printf("cannot create input memory buffer for boxes\n");
        return 1;
    }

    // copy to the respective memory buffers
    error = clEnqueueWriteBuffer(command_queue, buffer_boxes, CL_TRUE, 0,
          sizeof(float)*nmb_boxes_padding, boxes, 0, NULL, NULL);
    if (error != CL_SUCCESS) {
        printf("cannot write boxes to input buffer\n");
        return 1;
    }

    // set arguments of kernel
    error = clSetKernelArg(kernel, 0, sizeof(cl_mem), &buffer_boxes);
    if (error != CL_SUCCESS) {
        printf("cannot set kernel argument for buffer_boxes\n");
        return 1;
    }
    error = clSetKernelArg(kernel, 1, sizeof(float), &diffusion_const);
    if (error != CL_SUCCESS) {
        printf("cannot set kernel argument for diffusion_const\n");
        return 1;
    }

    // execute opencl kernel
    const size_t global[] = {width, height};
    for (size_t i = 0; i < iters; ++i) {
        error = clEnqueueNDRangeKernel(command_queue, kernel, 2, NULL,
            (const size_t *) &global, NULL, 0, NULL, NULL);
        if (error != CL_SUCCESS) {
            printf("cannot execute kernel\n");
            return 1;
        }
        // check status when debugging
        // status = clFlush(command_queue);
        // status = clFinish(command_queue);
    }

    // read buffer on device memory to host memory
    error = clEnqueueReadBuffer(command_queue, buffer_boxes, CL_TRUE, 0,
          sizeof(float)*nmb_boxes_padding, boxes, 0, NULL, NULL);
    if (error != CL_SUCCESS) {
        printf("cannot read from buffer\n");
        return 1;
    }
    // print out the result boxes
    // for (int h = 0; h < height; ++h) {
    //     for (int w = 0; w < width; ++w) {
    //         printf("%g ", boxes[h*width+w]);
    //     }
    //     printf("\n");
    // }

    // block until all previously queued opencl commands have completed
    status = clFinish(command_queue);

    avg = 0;
    for (size_t i = 0; i < nmb_boxes_padding; ++i)
        avg += boxes[i];
    avg /= nmb_boxes;
    
    avg_abs_diff = 0;
    float diff;
    for (size_t i = 0; i < nmb_boxes_padding; ++i) {
        diff = boxes[i] - avg;
        avg_abs_diff += ( (diff < 0) ? (-diff) : diff );
    }
    // substract the abs diff which come from padding
    avg_abs_diff -= (nmb_boxes_padding - nmb_boxes) * avg;
    avg_abs_diff /= nmb_boxes;

    // specifier %g use the shortest representation: %e or %f
    // print average temperature
    printf("average: %g\n", avg);
    // print average of absolute difference of each temperature and the average
    printf("average absolute difference: %g\n", avg_abs_diff);

    clReleaseMemObject(buffer_boxes);
    clReleaseKernel(kernel);
    clReleaseProgram(program);
    clReleaseCommandQueue(command_queue);
    clReleaseContext(context);
    free(opencl_program_src);
    free(boxes);
    return 0;
}