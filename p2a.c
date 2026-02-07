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
#define output_file "output1.raw"

double get_time() {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (double)tv.tv_sec + (double)tv.tv_usec * 1e-6;
}

float global_means[NUM_CLUSTERS] = {0.0f, 65.0f, 100.0f, 125.0f, 190.0f, 255.0f};

typedef struct {
    int thread_id;
    unsigned char* img_data;
    int start_idx;
    int end_idx;
    long local_sums[NUM_CLUSTERS];
    int local_counts[NUM_CLUSTERS];
} ThreadData;

void* assign_clusters_worker(void* arg) {
    ThreadData* data = (ThreadData*)arg;
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
    
    pthread_exit(NULL);
}

void* update_pixels_worker(void* arg) {
    ThreadData* data = (ThreadData*)arg;

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
   
    pthread_t* threads = (pthread_t*) malloc(num_threads * sizeof(pthread_t));
    ThreadData* t_args = (ThreadData*) malloc(num_threads * sizeof(ThreadData));

    int chunk_size = num_pixels / num_threads;

    double start_time = get_time();

    for (int iter = 0; iter < MAT_ITER; iter++) {
        for (int i = 0; i < num_threads; i++) {
            t_args[i].thread_id = i;
            t_args[i].img_data = a;
            
            t_args[i].start_idx = i * chunk_size;
            if (i == num_threads - 1) {
                t_args[i].end_idx = num_pixels;
            } else {
                t_args[i].end_idx = (i + 1) * chunk_size;
            }

            int rc = pthread_create(&threads[i], NULL, assign_clusters_worker, (void*)&t_args[i]);
            if (rc) {
                printf("Error: unable to create thread, %d\n", rc);
                exit(-1);
            }
        }

        for (int i = 0; i < num_threads; i++) {
            pthread_join(threads[i], NULL);
        }

        long total_sums[NUM_CLUSTERS] = {0};
        int total_counts[NUM_CLUSTERS] = {0};

        for (int i = 0; i < num_threads; i++) {
            for (int k = 0; k < NUM_CLUSTERS; k++) {
                total_sums[k] += t_args[i].local_sums[k];
                total_counts[k] += t_args[i].local_counts[k];
            }
        }

        for (int k = 0; k < NUM_CLUSTERS; k++) {
            if (total_counts[k] > 0) {
                global_means[k] = (float)total_sums[k] / total_counts[k];
            }
        }
    }

    for (int i = 0; i < num_threads; i++) {
        pthread_create(&threads[i], NULL, update_pixels_worker, (void*)&t_args[i]);
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
    free(t_args);

    return 0;
}