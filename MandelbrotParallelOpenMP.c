#include <stdio.h>
#include <stdlib.h>
#include <omp.h>
#include <time.h>

#define WIDTH 640
#define HEIGHT 480
#define MAX_ITER 255

// Define a complex number structure to represent each point in the Mandelbrot set
struct complex {
    double real;
    double imag;
};

// Function to calculate the iteration count for each pixel in the Mandelbrot set
int cal_pixel(struct complex c) {
    double z_real = 0;
    double z_imag = 0;
    double z_real2, z_imag2, lengthsq;
    int iter = 0;

    // Mandelbrot iteration loop
    do {
        z_real2 = z_real * z_real;
        z_imag2 = z_imag * z_imag;
        z_imag = 2 * z_real * z_imag + c.imag;
        z_real = z_real2 - z_imag2 + c.real;
        lengthsq = z_real2 + z_imag2;
        iter++;
    } while ((iter < MAX_ITER) && (lengthsq < 4.0));  // Stop if max iterations or escape radius is reached

    return iter;
}

// Function to save the computed Mandelbrot set image to a .pgm file
void save_pgm(const char *filename, int image[HEIGHT][WIDTH]) {
    FILE* pgmimg = fopen(filename, "wb");
    fprintf(pgmimg, "P2\n");
    fprintf(pgmimg, "%d %d\n", WIDTH, HEIGHT);
    fprintf(pgmimg, "255\n");

    // Write each pixel's intensity to the file
    for (int i = 0; i < HEIGHT; i++) {
        for (int j = 0; j < WIDTH; j++) {
            fprintf(pgmimg, "%d ", image[i][j]);
        }
        fprintf(pgmimg, "\n");
    }

    fclose(pgmimg);
}

int main() {
    int image[HEIGHT][WIDTH];
    double AVG = 0;
    int N = 10;
    double total_time[N];
    struct complex c;

    // Run 10 trials to calculate the average execution time
    for (int k = 0; k < N; k++) {
        double start_time = omp_get_wtime();

        // Parallelize the outer loop using OpenMP
        #pragma omp parallel for schedule(dynamic)
        // The `schedule(dynamic)` directive allows dynamic chunk assignment
        // of rows, enabling better load balancing for varying workloads per row.
        for (int i = 0; i < HEIGHT; i++) {
            for (int j = 0; j < WIDTH; j++) {
                // Map pixel coordinates to complex plane
                c.real = (j - WIDTH / 2.0) * 4.0 / WIDTH;
                c.imag = (i - HEIGHT / 2.0) * 4.0 / HEIGHT;
                
                // Calculate pixel intensity
                image[i][j] = cal_pixel(c);
            }
        }

        double end_time = omp_get_wtime();
        total_time[k] = end_time - start_time;

        // Print execution time for each trial
        printf("Execution time of trial [%d]: %f seconds\n", k, total_time[k]);
        AVG += total_time[k];
    }

    // Save the final Mandelbrot set image
    save_pgm("mandelbrot.pgm", image);
    
    // Calculate and print the average execution time over 10 trials
    printf("The average execution time of 10 trials is: %f ms\n", (AVG / N) * 1000);

    return 0;
}
