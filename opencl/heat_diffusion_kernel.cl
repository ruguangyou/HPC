__kernel void diffusion (__global float *boxes, float diffusion_const) {
    
    // width and height is the one with padding
    int width = get_global_size(0);
    int height = get_global_size(1);
    int ix = get_global_id(0);
    int jx = get_global_id(1);
    int kx = jx*width + ix;
    float value;

    // update temperature value
    if (ix == 0 || ix == width-1 || jx == 0 || jx == height-1)
        value = 0;
    else 
        value = boxes[kx] + diffusion_const * ( (boxes[kx-1]+boxes[kx+1]+boxes[kx-width]+boxes[kx+width])/4 - boxes[kx] );
    // If barrier is inside a loop, all work-items in a work-group must
    // execute the barrier for each iteration of the loop before they 
    // are allowed to continue execution beyond the barrier.
    barrier(CLK_GLOBAL_MEM_FENCE);
    boxes[kx] = value;
    // barrier(CLK_GLOBAL_MEM_FENCE);

    // The bug is: a work-group can synchronize internally, between work-items;
    //             but it cannot synchronize between work-groups.
    // In this program, it seems that there are more than one work-groups,
    // even with the barrier, the synchronization is still cannot be guarateed.
    // e.g. when updating the up-left corner box, the right side box has been
    //      updated, but the down side box hasn't.
    // So the way to solve is implementing a for-loop in host-side,
    // by iteratively invoking clEnqueueNDRangeKernel()
}