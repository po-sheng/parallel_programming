/**********************************************************************
 * DESCRIPTION:
 *   Serial Concurrent Wave Equation - C Version
 *   This program implements the concurrent wave equation
 *********************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <sys/time.h>

#define MAXPOINTS 1000000
#define MAXSTEPS 1000000
#define MINPOINTS 20
#define PI 3.14159265

void check_param(void);
void init_line(void);
void update (void);
void printfinal (void);

int nsteps,                     /* number of time steps */
    tpoints,                /* total points along string */
    rcode;                      /* generic return code */
float  values[MAXPOINTS+2],     /* values at time t */
       oldval[MAXPOINTS+2],     /* values at time (t-dt) */
       newval[MAXPOINTS+2];     /* values at time (t+dt) */


/**********************************************************************
 *  Checks input values from parameters
 *********************************************************************/
void check_param(void)
{
   char tchar[20];

   /* check number of points, number of iterations */
   while ((tpoints < MINPOINTS) || (tpoints > MAXPOINTS)) {
      printf("Enter number of points along vibrating string [%d-%d]: "
           ,MINPOINTS, MAXPOINTS);
      scanf("%s", tchar);
      tpoints = atoi(tchar);
      if ((tpoints < MINPOINTS) || (tpoints > MAXPOINTS))
         printf("Invalid. Please enter value between %d and %d\n", 
                 MINPOINTS, MAXPOINTS);
   }
   while ((nsteps < 1) || (nsteps > MAXSTEPS)) {
      printf("Enter number of time steps [1-%d]: ", MAXSTEPS);
      scanf("%s", tchar);
      nsteps = atoi(tchar);
      if ((nsteps < 1) || (nsteps > MAXSTEPS))
         printf("Invalid. Please enter value between 1 and %d\n", MAXSTEPS);
   }

   printf("Using points = %d, steps = %d\n", tpoints, nsteps);

}

/**********************************************************************
 *     Initialize points on line
 *********************************************************************/
void init_line(void)
{
   int i, j;
   float x, fac, k, tmp;

   /* Calculate initial values based on sine curve */
   fac = 2.0 * PI;
   k = 0.0; 
   tmp = tpoints - 1;
   for (j = 1; j <= tpoints; j++) {
      x = k/tmp;
      values[j] = sin(fac * x);
      k = k + 1.0;
   } 

   /* Initialize old values array */
   for (i = 1; i <= tpoints; i++) 
      oldval[i] = values[i];
}

/**********************************************************************
 *      Calculate new values using wave equation
 *********************************************************************/
// __device__ void do_math(int i)
// {
//    float dtime, c, dx, tau, sqtau;

//    dtime = 0.3;
//    c = 1.0;
//    dx = 1.0;
//    tau = (c * dtime / dx);
//    sqtau = tau * tau;
//    newval[i] = (2.0 * values[i]) - oldval[i] + (sqtau *  (-2.0)*values[i]);
// }

/**********************************************************************
*      Our Kernel
**********************************************************************/
__global__ void kernel(float *oldval, float *values, int tpoints, int nsteps)  {
    int idx = threadIdx.x;
    float old = oldval[idx+1], val = values[idx+1], newv = 0.0;
    float dtime = 0.3, c = 1.0, dx = 1.0, tau, sqtau;
    for(int i = 1; i <= nsteps; i++)  {
        /* global endpoints */
        if ((idx == 0) || (idx  == tpoints-1))
            newv = 0.0;
        else  {
            tau = (c * dtime / dx);
            sqtau = tau * tau;
            newv = (2.0 * val) - old + (sqtau *  (-2.0)*val);
        }
        
        /* Update old values with new values */
        old = val;
        val = newv;
    }
    values[idx+1] = val;
    __syncthreads();
}

/**********************************************************************
 *     Update all values along line a specified number of times
 *********************************************************************/
void update()
{
  int numBlocks = 1;
  int threadBlocks = tpoints;
  float *oldval_d, *values_d;
  cudaMalloc(&oldval_d, (tpoints+1)*sizeof(float));
  cudaMemcpy(oldval_d, values, (tpoints+1)*sizeof(float), cudaMemcpyHostToDevice);
  cudaMalloc(&values_d, (tpoints+1)*sizeof(float));
  cudaMemcpy(values_d, values, (tpoints+1)*sizeof(float), cudaMemcpyHostToDevice);
//  cudaMalloc(&newval_d, (tpoints+1)*sizeof(float));
//  cudaMemcpy(newval_d, newval, (tpoints+1)*sizeof(float), cudaMemcpyHostToDevice);

  kernel<<<numBlocks, threadBlocks>>>(oldval_d, values_d, tpoints, nsteps);

  cudaMemcpy(values, values_d, (tpoints+1)*sizeof(float), cudaMemcpyDeviceToHost);
  cudaFree(oldval_d);
  cudaFree(values_d);
//  cudaFree(newval_d);
}

/**********************************************************************
 *     Print final results
 *********************************************************************/
void printfinal()
{
   int i;

   for (i = 1; i <= tpoints; i++) {
      printf("%6.4f ", values[i]);
      if (i%10 == 0)
         printf("\n");
   }
}

/**********************************************************************
 *  Main program
 *********************************************************************/
int main(int argc, char *argv[])
{
//    struct timespec st, end;
//    clock_gettime(CLOCK_REALTIME, &st);

    sscanf(argv[1],"%d",&tpoints);
    sscanf(argv[2],"%d",&nsteps);
    check_param();
    printf("Initializing points on the line...\n");
    init_line();
    printf("Updating all points for all time steps...\n");
    update();
    printf("Printing final results...\n");
    printfinal();
    printf("\nDone.\n\n");
    
//    clock_gettime(CLOCK_REALTIME, &end);
//    double secs = (( double)end.tv_sec - (double)st.tv_sec ) + (( double)end.tv_nsec - (double)st.tv_nsec )/1000000000.0;
//    printf("Time: %.6f seconds\n", secs);
    return 0;
}
