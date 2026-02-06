#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <pthread.h>
#include <string.h> // For memset

// Global variables
int n = 2048; // Matrix size 2K x 2K
double **A, **B, **C;
int p; // Square root of total number of threads
pthread_mutex_t *row_mutexes; // Array of mutexes, one for each row of C

struct ThreadArgs {
    int thread_id;
};

void* matrix_multiplication_shared(void* args) {
    struct ThreadArgs* my_args = (struct ThreadArgs*) args;
    int id = my_args->thread_id;

    // Determine grid coordinates (row_grid, col_grid)
    int row_grid = id / p;
    int col_grid = id % p;

    // Block size
    int block_size = n / p;

    // Define the block of Matrix A this thread owns
    // Rows: [r_start, r_end)
    // Cols: [k_start, k_end) -> These correspond to rows of B we need
    int r_start = row_grid * block_size;
    int r_end = r_start + block_size;
    int k_start = col_grid * block_size;
    int k_end = k_start + block_size;

    // Allocate a temporary row buffer to accumulate partial results locally
    // This reduces the number of times we need to lock/unlock mutexes
    double* temp_row = (double*) malloc(sizeof(double) * n);

    int i, j, k;

    // Iterate over the rows of A assigned to this thread
    for (i = r_start; i < r_end; i++) {
        
        // 1. Clear temp buffer for the current row
        for(j = 0; j < n; j++) {
            temp_row[j] = 0.0;
        }

        // 2. Compute partial results for this row using the thread's block of A
        // We multiply A[i][k] (from our block) * B[k][j] (entire row of B)
        // Optimization: k loop is outer to reuse A[i][k]
        for (k = k_start; k < k_end; k++) {
            double a_val = A[i][k];
            for (j = 0; j < n; j++) {
                temp_row[j] += a_val * B[k][j];
            }
        }

        // 3. Update the global Matrix C in a Critical Section
        // We only lock the specific row 'i' we are updating
        pthread_mutex_lock(&row_mutexes[i]);
        for (j = 0; j < n; j++) {
            C[i][j] += temp_row[j];
        }
        pthread_mutex_unlock(&row_mutexes[i]);
    }

    free(temp_row);
    pthread_exit(NULL);
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Error: ./p1b {p}\n");
        return 1;
    }
    
    p = atoi(argv[1]);
    int num_threads = p * p;

    if (n % p != 0) {
        printf("Error: Matrix size %d must be divisible by p=%d\n", n, p);
        return 1;
    }

    int i, j;
    struct timespec start, stop;
    double time_taken;

    // Allocate matrices
    A = (double**) malloc(sizeof(double*) * n);
    B = (double**) malloc(sizeof(double*) * n);
    C = (double**) malloc(sizeof(double*) * n);
    for (i = 0; i < n; i++) {
        A[i] = (double*) malloc(sizeof(double) * n);
        B[i] = (double*) malloc(sizeof(double) * n);
        C[i] = (double*) malloc(sizeof(double) * n);
    }

    // Initialize matrices
    for (i = 0; i < n; i++) {
        for (j = 0; j < n; j++) {
            A[i][j] = i;
            B[i][j] = i + j;
            C[i][j] = 0;
        }
    }

    // Initialize Mutexes (One per row to minimize contention)
    row_mutexes = (pthread_mutex_t*) malloc(sizeof(pthread_mutex_t) * n);
    for(i = 0; i < n; i++) {
        pthread_mutex_init(&row_mutexes[i], NULL);
    }

    // Prepare threads
    pthread_t* threads = (pthread_t*) malloc(sizeof(pthread_t) * num_threads);
    struct ThreadArgs* thread_args = (struct ThreadArgs*) malloc(sizeof(struct ThreadArgs) * num_threads);

    // --- Start Timing ---
    if (clock_gettime(CLOCK_REALTIME, &start) == -1) { perror("clock gettime"); }

    for (i = 0; i < num_threads; i++) {
        thread_args[i].thread_id = i;
        int rc = pthread_create(&threads[i], NULL, matrix_multiplication_shared, (void *)&thread_args[i]);
        if (rc) {
            printf("Error: unable to create thread, %d\n", rc);
            exit(-1);
        }
    }

    for (i = 0; i < num_threads; i++) {
        pthread_join(threads[i], NULL);
    }

    // --- Stop Timing ---
    if (clock_gettime(CLOCK_REALTIME, &stop) == -1) { perror("clock gettime"); }
    time_taken = (stop.tv_sec - start.tv_sec) + (double)(stop.tv_nsec - start.tv_nsec) / 1e9;

    // Cleanup Mutexes
    for(i = 0; i < n; i++) {
        pthread_mutex_destroy(&row_mutexes[i]);
    }
    free(row_mutexes);

    // Output results
    printf("Execution time = %f sec\n", time_taken);
    printf("C[100][100]=%f\n", C[100][100]);

    // Free memory
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