#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <float.h>
#include <sys/time.h> // Changed from omp.h

#define h  800 
#define w  800
#define MAT_ITER 50 // Changed to 50 to match the assignment requirement

#define input_file  "input.raw"
#define output_file "output_serial.raw"

// --- Helper for timing ---
double get_time() {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (double)tv.tv_sec + (double)tv.tv_usec * 1e-6;
}

int main(int argc, char** argv){
    int i;
    FILE *fp;

    unsigned char *a = (unsigned char*) malloc (sizeof(unsigned char)*h*w);
    
    // Read Input
    if (!(fp=fopen(input_file, "rb"))) {
        printf("can not opern file\n");
        return 1;
    }
    fread(a, sizeof(unsigned char), w*h, fp);
    fclose(fp);
   
    // Start Timer
    double start_time = get_time();

    //  Your code goes here
    float means[6] = {0.0f, 65.0f, 100.0f, 125.0f, 190.0f, 255.0f}; // Updated to 6 clusters
    long sums[6];
    int counts[6];

    int num_pixels = w * h;

    for (int iter = 0; iter < MAT_ITER; iter++) {
        
        for (int k = 0; k < 6; k++) {
            sums[k] = 0;
            counts[k] = 0;
        }

        for (int j = 0; j < num_pixels; j++) {
            unsigned char pixel = a[j];
            float min_dist = FLT_MAX;
            int closest_cluster = -1;

            for (int k = 0; k < 6; k++) {
                float dist = fabsf(pixel - means[k]);
                if (dist < min_dist) {
                    min_dist = dist;
                    closest_cluster = k;
                }
            }
            sums[closest_cluster] += pixel;
            counts[closest_cluster]++;
        }

        for (int k = 0; k < 6; k++) {
            if (counts[k] > 0) {
                means[k] = (float)sums[k] / counts[k];
            }
        }
    }

    for (int j = 0; j < num_pixels; j++) {
        unsigned char pixel = a[j];
        float min_dist = FLT_MAX;
        int closest_cluster;

        for (int k = 0; k < 6; k++) {
            float dist = fabsf(pixel - means[k]);
            if (dist < min_dist) {
                min_dist = dist;
                closest_cluster = k;
            }
        }
        a[j] = (unsigned char)means[closest_cluster];
    }       
    
    // End Timer
    double end_time = get_time();
    
    printf("Time Execution: %f seconds\n", end_time - start_time);      
    
    if (!(fp=fopen(output_file,"wb"))) {
        printf("can not opern file\n");
        return 1;
    }   
    fwrite(a, sizeof(unsigned char),w*h, fp);
    fclose(fp);
    
    return 0;
}