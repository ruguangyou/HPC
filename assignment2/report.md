# Report of assignment 2, HPC



### Using POSIX threads

#### Newton' method





#### What I have done:

* Assumption: degree < 10, maximum iterations < 100

* Native implementation of arguments parsing (many if...else...)

* Pre-compute roots, stored in array ***roots_re*** and ***roots_im***, both have ***d*** elements

* Compute the coordinate of each point, stored in ***re_inits*** and ***im_inits***, both has ***num_lines*** elements

  ~~~c
  double interval = 4.0 / (num_lines - 1);
      double temp;
      for (size_t ix = 0; ix < num_lines; ++ix) {
          temp = -2 + interval * ix;
          re_inits[ix] = temp;
          im_inits[num_lines - 1 - ix] = temp;
      }
  ~~~

- Pre-set color string and gray string, which is used for writing attractor and convergence file respectively. One example for color string is "1 3 5 ", taking 6 chars (***len_color=6***, based on d < 10); for gray string is "25 25 25 ", which takes 9 char (***len_gray=9***, based on max_iter < 100)

  ~~~ c
  // preset colors
  // need d+1 kinds of color, each color has RGB value and corresponding space chars
  char *colors = (char *) malloc((d+1) * len_color * sizeof(char));
  color_sets = (char **) malloc((d+1) * sizeof(char *));
  for (i = 0; i <= d; ++i)
      color_sets[i] = colors + i*len_color;
  // initialize colors
  for (i = 0; i <= d; ++i)
      for (int j = 0; j < len_color; j += 2) {
          color_sets[i][j] = (i+j) % (d+1) + '0';
          color_sets[i][j+1] = ' ';
  }
  
  // preset grays
  // according to valgrind memcheck, when using sprintf the string should add one more byte for '\0' as the end
  char *grays = (char *) malloc(max_iter * (len_gray+1) * sizeof(char));
  gray_sets = (char **) malloc(max_iter * sizeof(char *));
  for (size_t i = 0; i < max_iter; ++i)
      gray_sets[i] = grays + i*len_gray;
  // initialize grays
  for (size_t i = 0; i < max_iter; ++i)
      sprintf(gray_sets[i], "%02zu %02zu %02zu ", i+1, i+1, i+1);
  
  ~~~

- Use char as the data type for attractor and convergence

  ~~~c
  // attractors is initialized to 0; 0 represents the calculation of this point
  // is not done yet; this setting would be useful in threads synchronization later
  attractor = (char *) calloc(num_items, sizeof(char));
  attractors = (char **) malloc(num_items * sizeof(char *));
  convergence = (char *) malloc(num_items * sizeof(char));
  convergences = (char **) malloc(num_items * sizeof(char *));
  for (i = 0; i < num_lines; ++i) {
      attractors[i] = attractor + i*num_lines;
      convergences[i] = convergence + i*num_lines;
  }
  ~~~

- Hardcode the complex number power formula in a separate *complex.c* file, taking cpx^6 as an example:

  ~~~c
  void complex_power_6 (double *cpx) {
      // (a+bi)^6 = ((a+bi)^2)^3 = ((a2+b2i)^2)*(a2+b2i) = (a4+b4i)*(a2+b2i)
      double a, b, a2, b2, a4, b4;
      a = cpx[0];
      b = cpx[1];
      a2 = a*a - b*b;
      b2 = 2*a*b;
      a4 = a2*a2 - b2*b2;
      b4 = 2*a2*b2;
      cpx[0] = a2*a4 - b2*b4;
      cpx[1] = a2*b4 + b2*a4;
  }
  ~~~

- Use a function pointer, and determine which formula to use before computing threads start

  ~~~ c
  void (*complex_power)(double *cpx);
  switch(d-1) {
      ...
      case 6:
          complex_power = complex_power_6;
          break;
      ...
  }
  ~~~

- Create threads with pthread_create, synchronize with pthread_join as well as pthread_mutex

- Compute the picture row by row, i.e. with a row as a work item in a thread

  ~~~c
  for (ix = offset; ix < num_lines; ix += num_threads) {
  	for (jx = 0; jx < num_lines; ++jx) {
          ...
      }
  }
  ~~~

- Compute one iteration of Newton's method with several steps

  ~~~ c
  // compute x^(d-1);
  cpx[0] = re_prev;
  cpx[1] = im_prev;
  complex_power(cpx);
  re_temp = cpx[0];
  im_temp = cpx[1];
  // x_next = x - f(x)/f'(x) = x - (x-1/x^(d-1))/d
  // 1/(a+bi) = (a-bi)/(a^2+b^2) = a*c-(b*c)i, c = 1/(a^2+b^2)
  temp = 1.0 / (re_temp*re_temp + im_temp*im_temp);
  re_next = re_prev - (re_prev - re_temp * temp) / d;
  im_next = im_prev - (im_prev + im_temp * temp) / d;
  ~~~

- Obtain absolute value without using abs(), judge distance to origin

  ~~~ c
  re_temp = (re_next < 0) ? (-re_next) : re_next;
  im_temp = (im_next < 0) ? (-im_next) : im_next;
  if (re_temp > 10000000000 || im_temp > 10000000000)
  	break;
                  
  temp = re_next*re_next + im_next*im_next;
  // sqrt(re^2 + im^2) < 10^-3, is just re^2 + im^2 < 10^-6
  	if (temp < 0.000001)
  		break;
  ~~~

- Compare with pre-computed roots in a for loop

  ~~~ c
  for (int n = 0; n < d; ++n) {
  	re_temp = re_next - roots_re[n];
      im_temp = im_next - roots_im[n];
      temp = re_temp*re_temp + im_temp*im_temp;
      if (temp < 0.000001) { 
      	attr = n + 1;
          conv = iter;
          break;
      }
  }
  ~~~

- The default value of ***attr*** is *d+1*, which represents the additional root; if converged to any one of the roots, ***attr*** will be set to *n+1*; 0 is reserved to represent that the calculation hasn't done

  ~~~ c
  if (attr != d+1)
      break;  // stop the iteration
  ~~~

- Use mutex lock to protect writing ***attr***; because in the write thread, the value of ***attr*** will be judged, we should lock it to avoid race condition

- In write thread, also work row by row. 

  Judge if the last point in a row has been done or not, and set ***row_done***. Note mutex lock is used here to make sure when reading ***attractors*** there is no thread write to is at the same time. 

  If a row is not done, sleep 100 micro seconds. 

  if a row is done, write to file using fwrite() and a for loop, and after that set ***row_done*** to 0

  ~~~ c
  for (ix = 0; ix < num_lines; ) {
  	pthread_mutex_lock(&mutex_item);
      if ( attractors[ix][num_lines-1] != 0 )
          row_done = 1;
      pthread_mutex_unlock(&mutex_item);
          
      if (row_done == 0) {
          // sleep write_to_disc thread to avoiding locking the mutex all the time
          nanosleep(&sleep_timespec, NULL);
          continue;
      }
      
      for (jx = 0; jx < num_lines; ++jx) {
          fwrite(color_sets[ attractors[ix][jx]-1 ], sizeof(char), len_color, fa);
          fwrite(gray_sets[ convergences[ix][jx] ], sizeof(char), len_gray, fc);
      }
      fwrite("\n", sizeof(char), 1, fa);
      fwrite("\n", sizeof(char), 1, fc);
      row_done = 0;
      ix++;
  }
  ~~~



#### Running results

* Three files: newton.c   complex.c   complex.h

* gcc flags: -O2 -lm -pthread

* valgrind memcheck looks good

  ![1539296848784](/home/ruguang/.config/Typora/typora-user-images/1539296848784.png)

* valgrind cachegrind also looks good

  ![1539296907933](/home/ruguang/.config/Typora/typora-user-images/1539296907933.png)

* I copy the check script to my laptop, and add one line to show running time for each case. 

  Here is what I got: pass the first 8 tests, but fail the second last test (0.01s !!!)

  ![running_result](/home/ruguang/.config/Typora/typora-user-images/1539296509200.png)

* The above result is got by setting max_iter=20, which makes the picture looks quite coarse

  ![1539297052982](/home/ruguang/.config/Typora/typora-user-images/1539297052982.png)

* If I set max_iter=50, I get a preciser picture

  ![1539297125916](/home/ruguang/.config/Typora/typora-user-images/1539297125916.png)

  But now I got worse outcome, runtime from 0.27s to 0.32s for the "-l1000 -t10 7" case

  ![1539297216027](/home/ruguang/.config/Typora/typora-user-images/1539297216027.png)

* Things got even worse when I try to check submission on gantenbein

  The setting is max_iter=20 (the coarse one), and with O2 optimization, but the running time was about 0.42s (on my laptop this number is 0.27s) !

  ![1539297617601](/home/ruguang/.config/Typora/typora-user-images/1539297617601.png)

  And If run several times, the runtime would sometimes be 0.37s

  ![1539297857626](/home/ruguang/.config/Typora/typora-user-images/1539297857626.png)



#### My question

* How could I optimize further?

  e.g. hardcode color string?  better compiler flags?  ...

* Why the performance differs so much between my laptop and gantenbein? It is because my program is poorly written or gantenbein was busy when I ran these checks?

* What if gantenbein is always busy, how can I pass the check submission (since I have to pass this before sending solutions) ?

* On my laptop, when I ran the 50000-line case, it gave "Segmentation fault (care dumped)". Is it because the memory was run out? Or other reasons?