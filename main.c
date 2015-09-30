#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <unistd.h>
#include <string.h>
#include <sys/time.h>

//Set up what to print
//#define time_measure_1_thread_print_per_interation
//#define time_measure_1_thread_print_per_test_cycle

//Collect statistics about time differences, only works when perftool_standard_test and perftool_custom_test are defined
#define need_statistic

//Set up test parameters
#define test_cycles 10
#define iterations 100
#define num_of_mallocs 5000

//What test case do we need to be analyzed by a performance tool
#define perftool_standard_test
#define perftool_custom_test

/* Prototypes for our hooks.  */
static void *old_malloc_hook;
static void *old_free_hook;

static void *my_free_hook_0 (void *, const void *);
static void *my_malloc_hook_0 (size_t, const void *);

static void *
my_malloc_hook_0 (size_t size, const void *caller)
{

    __malloc_hook  = old_malloc_hook;
    malloc(size);
    __malloc_hook = my_malloc_hook_0;

    return;
}


static void *
my_free_hook_0 (void *ptr, const void *caller)
{

    __free_hook = old_free_hook;
    free(ptr);
    __free_hook = my_free_hook_0;

    return;
}

int main()
{
    int err;
    int i,j,k = 0;
    int *pointer_main;

    long int sec_standard, total_usec_standard;
    long int sec_custom, total_usec_custom;
    double ratio, ratio_sum_interation, ratio_sum_test_cycle;

    struct timeval tval_before_standard, tval_after_standard, tval_result_standard;
    struct timeval tval_before_custom, tval_after_custom, tval_result_custom;

    old_malloc_hook = __malloc_hook;
    old_free_hook = __free_hook;

    printf("Started...\n");


    for(k=1;k<=test_cycles;k++)
    {
        for(j=1;j<=iterations;j++){


        #ifdef perftool_standard_test
            __malloc_hook  = old_malloc_hook;
            __free_hook = old_free_hook;

            gettimeofday(&tval_before_standard, NULL);

            for(i=1;i<=num_of_mallocs;i++) {
                pointer_main = (int *)malloc(sizeof(int));
                free(pointer_main);
            }

            gettimeofday(&tval_after_standard, NULL);

            timersub(&tval_after_standard, &tval_before_standard, &tval_result_standard);
        #endif // perftool_standard_test

        #ifdef perftool_custom_test

            __malloc_hook = my_malloc_hook_0;
            __free_hook = my_free_hook_0;

            gettimeofday(&tval_before_custom, NULL);

            for(i=1;i<=num_of_mallocs;i++) {
                pointer_main = (int *)malloc(sizeof(int));
                free(pointer_main);
            }

            gettimeofday(&tval_after_custom, NULL);

            timersub(&tval_after_custom, &tval_before_custom, &tval_result_custom);
        #endif // perftool_custom_test

        #ifdef need_statistic
            sec_standard = (long int)tval_result_standard.tv_sec;
            sec_custom = (long int)tval_result_custom.tv_sec;
            total_usec_standard = sec_standard * 10^6 + (long int)tval_result_standard.tv_usec;
            total_usec_custom = sec_custom * 10^6 + (long int)tval_result_custom.tv_usec;
            ratio = ((double)total_usec_custom) / ((double)total_usec_standard);
            ratio_sum_interation += ratio;
        #endif // need_statistic


            #ifdef time_measure_1_thread_print_per_interation
                printf("Time elapsed with STANDARD malloc: %ld.%06ld\n", (long int)tval_result_standard.tv_sec, (long int)tval_result_standard.tv_usec);
                printf("Time elapsed with CUSTOM malloc:   %ld.%06ld\n", (long int)tval_result_custom.tv_sec, (long int)tval_result_custom.tv_usec);
                printf("total_usec_standard = %ld\n",total_usec_standard);
                printf("total_usec_custom = %ld\n",total_usec_custom);
                printf("Time difference: %d ms\n",total_usec_custom-total_usec_standard);
                printf("Time needed: %1.6f more\n\n",ratio);
            #endif // time_measure_1_thread_print_per_interation

        }
    #ifdef time_measure_1_thread_print_per_test_cycle
        printf("Test cycle: %d, iterations: %d, allocations and frees per iteration: %d\n",k,iterations,num_of_mallocs);
        printf("In average %1.6f more time was needed with custom malloc and free hooks.\n\n",ratio_sum_interation/iterations);
    #endif // time_measure_1_thread_print_per_test_cycle

    #ifdef need_statistic
        ratio_sum_test_cycle += ratio_sum_interation;
        ratio_sum_interation = 0;
    #endif // need_statistic
    }
    #ifdef need_statistic
        printf("In average %1.6f more time was needed with custom malloc and free hooks during %d test cycles.\n\n",ratio_sum_test_cycle/(test_cycles*iterations),test_cycles);
    #endif // need_statistic


    printf("Finished!\n");

    return 0;
}
