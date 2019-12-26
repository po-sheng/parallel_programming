#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <time.h>
#include <sys/time.h>
#ifndef W
#define W 20 // Width
#endif

int main(int argc, char **argv) {
    int L = atoi(argv[1]); // Length
    int iteration = atoi(argv[2]); // Iteration
    srand(atoi(argv[3])); // Seed
    float d = (float) random() / RAND_MAX * 0.2; // Diffusivity
    int *temp = malloc(L*W*sizeof(int)); // Current temperature
    int *next = malloc(L*W*sizeof(int)); // Next time step
    int rank, p;
    struct timespec st, end;
    clock_gettime(CLOCK_REALTIME, &st);
    for (int i = 0; i < L; i++) {
        for (int j = 0; j < W; j++) {
            temp[i*W+j] = random()>>3;
        }
    }
    int count = 0, balance = 0;
    
    int balance_sum, tag = 0;
    MPI_Datatype row, last_row;
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &p);
    MPI_Status status;
    int s_row, e_row;
    s_row = (L/p) * rank;
    e_row = (rank == p-1)? L: (L/p) * (rank + 1);
    MPI_Type_contiguous((L/p)*W, MPI_INT, &last_row);
    MPI_Type_commit(&last_row);
    MPI_Type_contiguous((L-(p-1)*(L/p))*W, MPI_INT, &row);
    MPI_Type_commit(&row);
    while (iteration--) { // Compute with up, left, right, down points
        balance = 1;
        count++;
        for (int i = s_row; i < e_row; i++) {
            for (int j = 0; j < W; j++) {
                float t = temp[i*W+j] / d;
                t += temp[i*W+j] * -4;
                t += temp[(i - 1 < 0 ? 0 : i - 1) * W + j];
                t += temp[(i + 1 >= L ? i : i + 1)*W+j];
                t += temp[i*W+(j - 1 < 0 ? 0 : j - 1)];
                t += temp[i*W+(j + 1 >= W ? j : j + 1)];
                t *= d;
                next[i*W+j] = t ;
                if (next[i*W+j] != temp[i*W+j]) {
                    balance = 0;
                }
            }
        }
        
        for(int i = 0; i < p; i++)  {
            if(i == p-1)
                MPI_Bcast(&next[(L/p)*i*W], 1, last_row, i, MPI_COMM_WORLD);
            else
                MPI_Bcast(&next[(L/p)*i*W], 1, row, i, MPI_COMM_WORLD);
        }    

        MPI_Allreduce(&balance, &balance_sum, 1, MPI_INT, MPI_SUM, MPI_COMM_WORLD);
        if(balance_sum == p)    {
            break;
        }
        MPI_Barrier(MPI_COMM_WORLD);
        int *tmp = temp;
        temp = next;
        next = tmp;
    }
    if(rank == 0)   {
        int min = temp[0];
        for (int i = 0; i < L; i++) {
            for (int j = 0; j < W; j++) {
                if (temp[i*W+j] < min) {
                    min = temp[i*W+j];
                }
            }
        }
        clock_gettime(CLOCK_REALTIME, &end);
        double secs = ((double)end.tv_sec - (double)st.tv_sec) + ((double)end.tv_nsec - (double)st.tv_nsec)/1000000000.0;
        printf("time: %.6f\n", secs);
        printf("Size: %d*%d, Iteration: %d, Min Temp: %d\n", L, W, count, min);
    }
    MPI_Finalize();
return 0;
}
