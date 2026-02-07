#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <pthread.h>

int n = 2048;
double **A, **B, **C;
int p;
pthread_mutex_t *row_mutexes;

struct ThreadArgs {
    int thread_id;
};

void* matrix_multiplication_shared(void* args) {
    struct ThreadArgs* my_args = (struct ThreadArgs*) args;
    int id = my_args->thread_id;
    int row_grid = id / p;
    int col_grid = id % p;
    int block_size = n / p;

    int r_start = row_grid * block_size;
    int r_end = r_start + block_size;
    int k_start = col_grid * block_size;
    int k_end = k_start + block_size;

    double* temp_row = (double*) malloc(sizeof(double) * n);
    int i, j, k;
    for (i = r_start; i < r_end; i++) {
        for (j = 0; j < n; j++) {
            double sum = 0.0;
            for (k = k_start; k < k_end; k++) {
                sum += A[i][k] * B[k][j];
            }
            temp_row[j] = sum;
        }
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
    int i, j;
    struct timespec start, stop;
    double time_taken;

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

    row_mutexes = (pthread_mutex_t*) malloc(sizeof(pthread_mutex_t) * n);
    for(i = 0; i < n; i++) {
        pthread_mutex_init(&row_mutexes[i], NULL);
    }

    pthread_t* threads = (pthread_t*) malloc(sizeof(pthread_t) * num_threads);
    struct ThreadArgs* thread_args = (struct ThreadArgs*) malloc(sizeof(struct ThreadArgs) * num_threads);

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

    if (clock_gettime(CLOCK_REALTIME, &stop) == -1) { perror("clock gettime"); }
    time_taken = (stop.tv_sec - start.tv_sec) + (double)(stop.tv_nsec - start.tv_nsec) / 1e9;

    for(i = 0; i < n; i++) {
        pthread_mutex_destroy(&row_mutexes[i]);
    }
    free(row_mutexes);

    printf("Execution time = %f sec\n", time_taken);
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