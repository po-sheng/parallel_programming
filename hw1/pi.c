#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>

int main(int argc, char **argv)
{

    unsigned long long in = 1;
    unsigned long long total = 2;
    double tol , change, new, secs , old = 0.0;
    struct timeval start , end;
    int threads ; /* ignored */

    if ( argc < 2) {
        exit (-1);
    }

    threads = atoi ( argv[1]);
    tol = atof ( argv[2]);
    if (( threads < 1) || ( tol < 0.0)) {
        exit (-1);
    }

    srand48(clock());
    gettimeofday (&start , NULL);
    for(int i=0; i<tol; i++)    {
        double x, y;
        x = drand48();
        y = drand48();
        total ++;
        if (( x*x + y*y) <= 1.00)
            in ++;
        new = 4.0 * (double)in/( double)total ;
    }
    gettimeofday (&end, NULL);
    secs = (( double)end.tv_sec - (double)start.tv_sec )
    + (( double)end.tv_usec - (double)start.tv_usec )/1000000.0;
    printf ("Found estimate of pi of %.12f in %llu iterations , %.6f seconds.\n",
            new, total - 2, secs );
}
