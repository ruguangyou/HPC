void diffusion (float *boxes, float *out,
           int width,  int height, float diffusion_const, int iters) {
    float value;

    for (int ix = 0; ix < height; ++ix) {
        for (int jx = 0; jx < width; ++jx) {
        // update temperature value
            int kx = ix*width + jx;
            if (ix == 0) {
                if (jx == 0)
                    value = boxes[kx] + diffusion_const * ( (boxes[kx+1]+boxes[kx+width])/4 - boxes[kx] );
                else if (jx == width-1)
                    value = boxes[kx] + diffusion_const * ( (boxes[kx-1]+boxes[kx+width])/4 - boxes[kx] );
                else
                    value = boxes[kx] + diffusion_const * ( (boxes[kx-1]+boxes[kx+1]+boxes[kx+width])/4 - boxes[kx] );
            }
            else if (ix == height-1) {
                if (jx == 0)
                    value = boxes[kx] + diffusion_const * ( (boxes[kx+1]+boxes[kx-width])/4 - boxes[kx] );
                else if (jx == width-1)
                    value = boxes[kx] + diffusion_const * ( (boxes[kx-1]+boxes[kx-width])/4 - boxes[kx] );
                else
                    value = boxes[kx] + diffusion_const * ( (boxes[kx-1]+boxes[kx+1]+boxes[kx-width])/4 - boxes[kx] );
            }
            else
                value = boxes[kx] + diffusion_const * ( (boxes[kx-1]+boxes[kx+1]+boxes[kx-width]+boxes[kx+width])/4 - boxes[kx] );
            
            out[kx] = value;
        }
    }
}