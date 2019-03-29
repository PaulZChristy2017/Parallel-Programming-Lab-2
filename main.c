#include <stdlib.h>
#include <stdio.h>
#include <omp.h>
#include <time.h>
#include <sys/time.h>

static const long Num_To_Add = 100000000;
static const double Scale = 10.0 / RAND_MAX;

long add_serial(const char *numbers) {
    long sum = 0;
    for (long i = 0; i < Num_To_Add; i++) {
        sum += numbers[i];
    }
    return sum;
}

long add_parallel(const char *numbers) {
/*

     * my_sum = 0;
     * my_first_i = . . . ;
     * my_last = . . . ;
     * for ( my_first_i ; my_i < my_last_i; my_i++ ) {
     *      my_x = Compute_next_value( . . . );
     * }
     *
     * if( I'm the master core ) {
     *      sum = my_x;
     *      for each other core other than myself {
     *          receive value from core;
     *          sum += value;
     *      }
     * }
     *
     * else {
     *      send my_x to the master;
     * }
*/

    // size per thread
    long chunk_size = Num_To_Add / omp_get_max_threads();

    char results[omp_get_max_threads()]; // size per thread

    long tid = omp_get_thread_num();
    printf("tid: %ld\n", tid);

    long my_sum = 0;

    //printf("numbers[0]: %ld\n", numbers[0]);
    //printf("numbers[1]: %ld\n", numbers[1]);

    // my_First_index = thread_id * size per thread
    long my_First_index = tid * chunk_size;
    printf("myF: %ld\n", my_First_index);

    // my_Last_index = my_First_index + size per thread
    long my_Last_index = my_First_index + chunk_size;
    printf("myL: %ld\n", my_Last_index);

    for ( long index = my_First_index; index < my_Last_index; index++ ) {
        long my_sum = my_sum + numbers[index];
    }

    printf("my_sum: %ld\n", my_sum);

    results[tid] = my_sum;

    printf("Max Threads: %d\n", omp_get_max_threads());

    long sum = my_sum;

    return sum;

}

int main() {
    char *numbers = malloc(sizeof(long) * Num_To_Add);
//    for(long i = 0; i < Num_To_Add; i++){
//        numbers[i] = (char) (rand() * Scale);
//        printf("Timing sequential...\n");
//    }

    long chunk_size = Num_To_Add / omp_get_max_threads();
#pragma omp parallel num_threads(omp_get_max_threads())
    {
        int p = omp_get_thread_num();
        unsigned int seed = (unsigned int) time(NULL) + (unsigned int) p;
        long chunk_start = p * chunk_size;
        long chunk_end = chunk_start + chunk_size;
        for (long i = chunk_start; i < chunk_end; i++) {
            numbers[i] = (char) (rand_r(&seed) * Scale);
        }
    }

    struct timeval start, end;

    printf("Timing sequential...\n");
    gettimeofday(&start, NULL);
    long sum_s = add_serial(numbers);
    gettimeofday(&end, NULL);
    printf("Took %f seconds\n\n", end.tv_sec - start.tv_sec + (double) (end.tv_usec - start.tv_usec) / 1000000);

    printf("Timing parallel...\n");
    gettimeofday(&start, NULL);
    long sum_p = add_parallel(numbers);
    gettimeofday(&end, NULL);
    printf("Took %f seconds\n\n", end.tv_sec - start.tv_sec + (double) (end.tv_usec - start.tv_usec) / 1000000);

    printf("Sum serial: %ld\nSum parallel: %ld", sum_s, sum_p);

    free(numbers);
    return 0;
}

