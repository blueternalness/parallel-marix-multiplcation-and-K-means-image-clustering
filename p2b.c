#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <float.h>
#include <pthread.h>
#include <sys/time.h>

#define h  800 
#define w  800
#define NUM_CLUSTERS 6
#define MAT_ITER 50

#define input_file  "input.raw"
#define output_file "output2.raw"

float global_means[NUM_CLUSTERS] = {0.0f, 65.0f, 100.0f, 125.0f, 190.0f, 255.0f};

pthread_mutex_t mutex;
pthread_cond_t cond_var;
int r = 0;

typedef struct {
    int thread_id;
    int num_threads;
    unsigned char* img_data;
    int start_idx;
    int end_idx;
    long local_sums[NUM_CLUSTERS];
    int local_counts[NUM_CLUSTERS];
} ThreadData;

ThreadData* all_thread_args;

double get_time() {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (double)tv.tv_sec + (double)tv.tv_usec * 1e-6;
}

void* thread_worker(void* arg) {
    ThreadData* data = (ThreadData*)arg;
    int p = data->num_threads;

    for (int iter = 0; iter < MAT_ITER; iter++) {
        for (int k = 0; k < NUM_CLUSTERS; k++) {
            data->local_sums[k] = 0;
            data->local_counts[k] = 0;
        }

        for (int i = data->start_idx; i < data->end_idx; i++) {
            unsigned char pixel = data->img_data[i];
            float min_dist = FLT_MAX;
            int closest_cluster = -1;

            for (int k = 0; k < NUM_CLUSTERS; k++) {
                float dist = fabsf(pixel - global_means[k]);
                if (dist < min_dist) {
                    min_dist = dist;
                    closest_cluster = k;
                }
            }
            data->local_sums[closest_cluster] += pixel;
            data->local_counts[closest_cluster]++;
        }

        pthread_mutex_lock(&mutex);
        
        if (r < p - 1) {
            r++; 
            pthread_cond_wait(&cond_var, &mutex);
        } else {
            long total_sums[NUM_CLUSTERS] = {0};
            int total_counts[NUM_CLUSTERS] = {0};

            for (int t = 0; t < p; t++) {
                for (int k = 0; k < NUM_CLUSTERS; k++) {
                    total_sums[k] += all_thread_args[t].local_sums[k];
                    total_counts[k] += all_thread_args[t].local_counts[k];
                }
            }

            for (int k = 0; k < NUM_CLUSTERS; k++) {
                if (total_counts[k] > 0) {
                    global_means[k] = (float)total_sums[k] / total_counts[k];
                }
            }

            r = 0;
            pthread_cond_broadcast(&cond_var);
        }
        
        pthread_mutex_unlock(&mutex);
    }

    for (int i = data->start_idx; i < data->end_idx; i++) {
        unsigned char pixel = data->img_data[i];
        float min_dist = FLT_MAX;
        int closest_cluster = -1;

        for (int k = 0; k < NUM_CLUSTERS; k++) {
            float dist = fabsf(pixel - global_means[k]);
            if (dist < min_dist) {
                min_dist = dist;
                closest_cluster = k;
            }
        }
        data->img_data[i] = (unsigned char)global_means[closest_cluster];
    }

    pthread_exit(NULL);
}

int main(int argc, char** argv) {
    if (argc < 2) {
        printf("Usage: %s <number_of_threads>\n", argv[0]);
        return 1;
    }

    int num_threads = atoi(argv[1]);
    int num_pixels = w * h;
    FILE *fp;

    unsigned char *a = (unsigned char*) malloc(sizeof(unsigned char) * num_pixels);
    
    if (!(fp = fopen(input_file, "rb"))) {
        printf("Cannot open file %s\n", input_file);
        return 1;
    }
    fread(a, sizeof(unsigned char), num_pixels, fp);
    fclose(fp);

    pthread_mutex_init(&mutex, NULL);
    pthread_cond_init(&cond_var, NULL);

    pthread_t* threads = (pthread_t*) malloc(num_threads * sizeof(pthread_t));
    all_thread_args = (ThreadData*) malloc(num_threads * sizeof(ThreadData));

    int chunk_size = num_pixels / num_threads;

    double start_time = get_time();

    for (int i = 0; i < num_threads; i++) {
        all_thread_args[i].thread_id = i;
        all_thread_args[i].num_threads = num_threads;
        all_thread_args[i].img_data = a;
        
        all_thread_args[i].start_idx = i * chunk_size;
        if (i == num_threads - 1) {
            all_thread_args[i].end_idx = num_pixels;
        } else {
            all_thread_args[i].end_idx = (i + 1) * chunk_size;
        }

        pthread_create(&threads[i], NULL, thread_worker, (void*)&all_thread_args[i]);
    }

    for (int i = 0; i < num_threads; i++) {
        pthread_join(threads[i], NULL);
    }

    double end_time = get_time();
    printf("Time Execution: %f seconds\n", end_time - start_time);

    if (!(fp = fopen(output_file, "wb"))) {
        printf("Cannot open file %s\n", output_file);
        return 1;
    }   
    fwrite(a, sizeof(unsigned char), num_pixels, fp);
    fclose(fp);

    free(a);
    free(threads);
    free(all_thread_args);
    pthread_mutex_destroy(&mutex);
    pthread_cond_destroy(&cond_var);

    return 0;
}