#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <pthread.h>

int n = 2048;
double **A, **B, **C;
int p;

struct ThreadArgs {
    int thread_id;
};

void* matrix_multiplication(void* args) {
    struct ThreadArgs* my_args = (struct ThreadArgs*) args;
    int id = my_args->thread_id;

    int row_grid = id / p;
    int col_grid = id % p;

    int slice = n / p;

    int r_start = row_grid * slice;
    int r_end = r_start + slice;
    int c_start = col_grid * slice;
    int c_end = c_start + slice;

    int i, j, k;
    for (i = r_start; i < r_end; i++) {
        for (j = c_start; j < c_end; j++) {
            double sum = 0;
            for (k = 0; k < n; k++) {
                sum += A[i][k] * B[k][j];
            }
            C[i][j] = sum;
        }
    }

    pthread_exit(NULL);
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Error: ./p1b {p}\n");
        printf("Example: ./p1b 4\n");
        return 1;
    }
    
    p = atoi(argv[1]);
    int num_threads = p * p;

    int i, j;
    struct timespec start, stop;
    double time;

    A = (double**) malloc(sizeof(double*) * n);
    B = (double**) malloc(sizeof(double*) * n);
    C = (double**) malloc(sizeof(double*) * n);
    for (i = 0; i < n; i++) {
        A[i] = (double*) malloc(sizeof(double) * n);
        B[i] = (double*) malloc(sizeof(double) * n);
        C[i] = (double*) malloc(sizeof(double) * n);
    }

    for (i = 0; i < n; i++) {
        for (j = 0; j < n; j++) {
            A[i][j] = i;
            B[i][j] = i + j;
            C[i][j] = 0;
        }
    }

    pthread_t* threads = (pthread_t*) malloc(sizeof(pthread_t) * num_threads);
    struct ThreadArgs* thread_args = (struct ThreadArgs*) malloc(sizeof(struct ThreadArgs) * num_threads);

    if (clock_gettime(CLOCK_REALTIME, &start) == -1) { perror("clock gettime"); }

    for (i = 0; i < num_threads; i++) {
        thread_args[i].thread_id = i;
        pthread_create(&threads[i], NULL, matrix_multiplication, (void *)&thread_args[i]);
    }

    for (i = 0; i < num_threads; i++) {
        pthread_join(threads[i], NULL);
    }

    if (clock_gettime(CLOCK_REALTIME, &stop) == -1) { perror("clock gettime"); }
    time = (stop.tv_sec - start.tv_sec) + (double)(stop.tv_nsec - start.tv_nsec) / 1e9;

    printf("Execution time = %f sec\n", time);
    printf("C[100][100]=%f\n", C[100][100]);

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