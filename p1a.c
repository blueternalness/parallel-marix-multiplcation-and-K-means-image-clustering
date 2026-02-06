#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <pthread.h>

// Global variables shared by threads
int n = 2048; // Matrix size is 4K * 4K
double **A, **B, **C;
int p; // Square root of total number of threads

// Struct to pass thread ID
struct ThreadArgs {
    int thread_id;
};

// The worker function for each thread
void* matrix_multiplication(void* args) {
    struct ThreadArgs* my_args = (struct ThreadArgs*) args;
    int id = my_args->thread_id;

    // Determine the grid coordinates (row, col) of the thread in the p x p grid
    // For example, if p=4, ID 5 is at row 1, col 1 (indices 0-3)
    int row_grid = id / p;
    int col_grid = id % p;

    // Calculate the size of the block each thread handles
    int slice = n / p;

    // Calculate the start and end indices for this thread's work
    int r_start = row_grid * slice;
    int r_end = r_start + slice;
    int c_start = col_grid * slice;
    int c_end = c_start + slice;

    // Perform computation for the assigned sub-block of C
    int i, j, k;
    for (i = r_start; i < r_end; i++) {
        for (j = c_start; j < c_end; j++) {
            double sum = 0;
            // Note: We need the full row of A and full column of B to compute C[i][j]
            for (k = 0; k < n; k++) {
                sum += A[i][k] * B[k][j];
            }
            C[i][j] = sum;
        }
    }

    pthread_exit(NULL);
}

int main(int argc, char *argv[]) {
    // Instruction: Pass the square root of total number of threads p
    if (argc < 2) {
        printf("Error: ./p1b {p}\n");
        printf("Example: ./p1b 4  (for 16 threads)\n");
        return 1;
    }
    
    p = atoi(argv[1]);
    int num_threads = p * p;

    // Validation check
    if (n % p != 0) {
        printf("Error: Matrix size %d must be divisible by p=%d\n", n, p);
        return 1;
    }

    int i, j;
    struct timespec start, stop;
    double time;

    // Allocate memory
    A = (double**) malloc(sizeof(double*) * n);
    B = (double**) malloc(sizeof(double*) * n);
    C = (double**) malloc(sizeof(double*) * n);
    for (i = 0; i < n; i++) {
        A[i] = (double*) malloc(sizeof(double) * n);
        B[i] = (double*) malloc(sizeof(double) * n);
        C[i] = (double*) malloc(sizeof(double) * n);
    }

    // Initialize matrices (Same logic as PHW1)
    for (i = 0; i < n; i++) {
        for (j = 0; j < n; j++) {
            A[i][j] = i;
            B[i][j] = i + j;
            C[i][j] = 0;
        }
    }

    // Thread management arrays
    pthread_t* threads = (pthread_t*) malloc(sizeof(pthread_t) * num_threads);
    struct ThreadArgs* thread_args = (struct ThreadArgs*) malloc(sizeof(struct ThreadArgs) * num_threads);

    // Start timing
    if (clock_gettime(CLOCK_REALTIME, &start) == -1) { perror("clock gettime"); }

    // ******************************* //
    // Create Threads
    for (i = 0; i < num_threads; i++) {
        thread_args[i].thread_id = i;
        int rc = pthread_create(&threads[i], NULL, matrix_multiplication, (void *)&thread_args[i]);
        if (rc) {
            printf("Error: unable to create thread, %d\n", rc);
            exit(-1);
        }
    }

    // Join Threads (Wait for all to finish)
    for (i = 0; i < num_threads; i++) {
        pthread_join(threads[i], NULL);
    }
    // ******************************* //

    // Stop timing
    if (clock_gettime(CLOCK_REALTIME, &stop) == -1) { perror("clock gettime"); }
    time = (stop.tv_sec - start.tv_sec) + (double)(stop.tv_nsec - start.tv_nsec) / 1e9;

    // Output requirements
    printf("Execution time = %f sec\n", time);
    printf("C[100][100]=%f\n", C[100][100]);

    // Release memory
    for (i = 0; i < n; i++) {
        free(A[i]);
        free(B[i]);
        free(C[i]);
    }
    free(A);
    free(B);
    free(C);
    free(threads);
    free(thread_args);

    return 0;
}