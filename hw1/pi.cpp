#include <iostream>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>

using namespace std;

// use semaphore to dynamically allocate resource to free thread
sem_t add;    
unsigned long long total = 2, in = 1;

// implement computation
void *dart(void *argument)    {
    int *input = (int *)argument;
    unsigned long long loc_in = 0;
    double x, y;
    // srand(input[1]);
    for(int i = 0; i < input[0]; i++)    {

        x = drand48();
        y = drand48();
        if (( x*x + y*y) <= 1.00)   {
            loc_in++;
        }
    }
    // sem_wait(&add);
    in += loc_in;
    // sem_post(&add);
    return NULL;
} 

int main(int argc, char **argv)
{
 
    double tol, pi, secs;
    struct timeval start , end;
    int threads ; /* ignored */

    if ( argc < 2) {
        exit (-1);
    }

    threads = atoi ( argv[1]);
    tol = atof ( argv[2]);
    int data[threads][2];
    total = tol;
    if (( threads < 1) || ( tol < 0.0)) {
        exit (-1);
    }

    for(int i = 0; i < threads; i++)    {
        data[i][1] = i+1;
        if(i != threads - 1) 
            data[i][0] = tol/threads;
        else 
            data[i][0] = tol - (threads - 1) * (tol/threads);
    }

    pthread_t thread[threads];
    sem_init(&add, 0, 1);

    srand48(clock());
    gettimeofday (&start , NULL);

    for(int i = 0; i < threads; i++)    
        pthread_create(&thread[i], NULL, dart, &data[i]);

    for(int i = 0; i < threads; i++)
        pthread_join(thread[i], NULL);

    pi = 4.0 * (double)in/( double)total ;

    gettimeofday (&end, NULL);
    secs = (( double)end.tv_sec - (double)start.tv_sec )
    + (( double)end.tv_usec - (double)start.tv_usec )/1000000.0;
    printf ("Found estimate of pi of %.12f in %llu iterations , %.6f seconds.\n",
            pi, total, secs );
}
