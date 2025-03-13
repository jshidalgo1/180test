#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <sys/time.h>
#include <pthread.h>
#include <sched.h>
#include <unistd.h>

// Mutex for thread-safe addition
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

// Structure to hold data for each thread
typedef struct {
    int **X;
    int *y;
    double *e; // MSE vector
    int start_row;
    int end_row;
    int n;
    int m;
    int core_id; // Core to assign thread
} ThreadData;

// Function to get the current time in seconds
double get_time() {
    struct timeval t;
    gettimeofday(&t, NULL);
    return t.tv_sec + t.tv_usec / 1000000.0;
}

// Thread function to compute MSE
void *mse_thread(void *arg) {
    ThreadData *data = (ThreadData *)arg;

    // Assign thread to a specific core
    cpu_set_t cpuset;
    CPU_ZERO(&cpuset);
    CPU_SET(data->core_id, &cpuset);
    pthread_setaffinity_np(pthread_self(), sizeof(cpu_set_t), &cpuset);

    for (int j = 0; j < data->n; j++) {
        double sum = 0.0;
        for (int i = data->start_row; i < data->end_row; i++) {
            sum += pow(data->X[i][j] - data->y[i], 2);
        }

        // Ensure thread-safe addition
        pthread_mutex_lock(&mutex);
        data->e[j] += sum;
        pthread_mutex_unlock(&mutex);
    }
    pthread_exit(NULL);
}

// Function to generate a random n x n matrix
int **generate_matrix(int size) {
    int **matrix = (int **)malloc(size * sizeof(int *));
    for (int i = 0; i < size; i++) {
        matrix[i] = (int *)malloc(size * sizeof(int));
        for (int j = 0; j < size; j++) {
            matrix[i][j] = (rand() % 10) + 1;
        }
    }
    return matrix;
}

// Function to generate a random n x 1 vector
int *generate_vector(int size) {
    int *vector = (int *)malloc(size * sizeof(int));
    for (int i = 0; i < size; i++) {
        vector[i] = (rand() % 10) + 1;
    }
    return vector;
}

int main() {
    double t1, t2;
    double time;

    int size, num_threads;
    printf("Enter matrix size: ");
    scanf("%d", &size);
    printf("Enter number of threads: ");
    scanf("%d", &num_threads);

    int **mat_ptr = generate_matrix(size);
    int *vec_ptr = generate_vector(size);
    double *results = (double *)calloc(size, sizeof(double));

    pthread_t threads[num_threads];
    ThreadData thread_data[num_threads];
    int rows_per_thread = size / num_threads;

    t1 = get_time();

    for (int i = 0; i < num_threads; i++) {
        thread_data[i].X = mat_ptr;
        thread_data[i].y = vec_ptr;
        thread_data[i].e = results;
        thread_data[i].n = size;
        thread_data[i].m = size;
        thread_data[i].start_row = i * rows_per_thread;
        thread_data[i].end_row = (i == num_threads - 1) ? size : (i + 1) * rows_per_thread;
        thread_data[i].core_id = i % sysconf(_SC_NPROCESSORS_ONLN); // Assign core

        pthread_create(&threads[i], NULL, mse_thread, &thread_data[i]);
    }

    for (int i = 0; i < num_threads; i++) {
        pthread_join(threads[i], NULL);
    }

    for (int j = 0; j < size; j++) {
        results[j] = sqrt(results[j] / size);
    }

    t2 = get_time();
    time = t2 - t1;

    printf("MSE Vector:\n");
    for (int i = 0; i < size; i++) {
        printf("%.2f\n", results[i]);
    }
    printf("\nTime Elapsed: %f seconds\n", time);

    for (int i = 0; i < size; i++) {
        free(mat_ptr[i]);
    }
    free(mat_ptr);
    free(vec_ptr);
    free(results);

    return 0;
}
