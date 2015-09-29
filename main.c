#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>
#include <sys/time.h>
#include <semaphore.h>

#define time_measure_1_thread
//#define thread_safe
//#define modify_malloc_hook

//#define time_measure_1_thread_print
#define iterations 100
#define num_of_mallocs 10000

/* Prototypes for our hooks.  */
static void *old_malloc_hook;
static void *my_malloc_hook_0 (size_t, const void *);
static void *my_malloc_hook_1 (size_t, const void *);
static void *my_malloc_hook_2 (size_t, const void *);


static void *old_free_hook;
static void *my_free_hook_0 (void *, const void *);

static int hook_counter = 0;

static sem_t mutex;

pthread_t tid[3];

void* Thread_0(void *arg)
{
    int i=0;
    int *pointer_0;

    int argument = (int)arg;

    __malloc_hook = my_malloc_hook_0;
    __free_hook = my_free_hook_0;

    for(i=0;i<10;i++)
    {

        printf("Thread %d, malloc and free %d\n",argument,i);
        pointer_0 = (int *)malloc(sizeof(int));
        free(pointer_0);

        #ifdef thread_safe
            usleep(500);
        #endif // thread_safe
    }

    return;

}

void* Thread_1(void *arg)
{
    printf("Thread 1 malloc\n");
    __malloc_hook = my_malloc_hook_1;
    int *pointer_1 = (int *)malloc(sizeof(int));
    *pointer_1 = 1;
    printf("*pointer_1 = %d\n",*pointer_1);

    __malloc_hook = my_malloc_hook_2;
    int *pointer_2 = (int *)malloc(sizeof(int));
    *pointer_2 = 2;
    printf("*pointer_2 = %d\n",*pointer_2);

    return NULL;
}

void* Thread_2(void *arg)
{
    printf("Thread 2 malloc\n");
    __malloc_hook = my_malloc_hook_1;
    int *pointer_1 = (int *)malloc(sizeof(int));
    *pointer_1 = 1;
    printf("*pointer_1 = %d\n",*pointer_1);

    __malloc_hook = my_malloc_hook_2;
    int *pointer_2 = (int *)malloc(sizeof(int));
    *pointer_2 = 2;
    printf("*pointer_2 = %d\n",*pointer_2);

    return NULL;
}




static void *
my_malloc_hook_0 (size_t size, const void *caller)
{
    sem_wait(&mutex);

    __malloc_hook  = old_malloc_hook;
    malloc(size);
    __malloc_hook = my_malloc_hook_0;
    hook_counter++;
    #ifdef thread_safe
      printf("hook_counter = %d\n",hook_counter);
    #endif

    sem_post(&mutex);

    return;
}

static void *
my_free_hook_0 (void *ptr, const void *caller)
{
    sem_wait(&mutex);

    __free_hook = old_free_hook;
    free(ptr);
    __free_hook = my_free_hook_0;
    hook_counter--;
    #ifdef thread_safe
      printf("hook_counter = %d\n",hook_counter);
    #endif

    sem_post(&mutex);

    return;
}

static void *
my_malloc_hook_1 (size_t size, const void *caller)
{
  printf("Malloc hook 1\n");
  __malloc_hook  = old_malloc_hook;
  malloc(size);
  __malloc_hook = my_malloc_hook_1;
  return;
}

static void *
my_malloc_hook_2 (size_t size, const void *caller)
{
  printf("Malloc hook 2\n");
  __malloc_hook  = old_malloc_hook;
  malloc(size);
  __malloc_hook = my_malloc_hook_2;
  return;
}


int main()
{
    int err;
    int i,j = 0;
    int *pointer_main;

    struct timeval tval_before_standard, tval_after_standard, tval_result_standard;
    struct timeval tval_before_custom, tval_after_custom, tval_result_custom;

    old_malloc_hook = __malloc_hook;
    old_free_hook = __free_hook;

    printf("Started...\n");

    sem_init(&mutex, 0, 1);


#ifdef thread_safe

    err = pthread_create(&(tid[0]), NULL, &Thread_0, 1);
    if (err != 0)
        printf("\ncan't create thread :[%s]", strerror(err));
    else
        printf("\n Thread %d created successfully\n",(int)tid[0]);

    printf("\n Created Thread 1\n");

    err = pthread_create(&(tid[1]), NULL, &Thread_0, 2);
    if (err != 0)
        printf("\ncan't create thread :[%s]", strerror(err));
    else
        printf("\n Thread %d created successfully\n",(int)tid[1]);

    printf("\n Created Thread 2\n");


#endif // thread_safe

#ifdef time_measure_1_thread

    long int sec_standard, total_usec_standard;
    long int sec_custom, total_usec_custom;
    double ratio, ratio_sum;

    for(j=0;j<iterations;j++){

        gettimeofday(&tval_before_standard, NULL);

        for(i=0;i<num_of_mallocs;i++) {
                pointer_main = (int *)malloc(sizeof(int));
                free(pointer_main);
        }

        gettimeofday(&tval_after_standard, NULL);

        timersub(&tval_after_standard, &tval_before_standard, &tval_result_standard);




        __malloc_hook = my_malloc_hook_0;
        __free_hook = my_free_hook_0;
        gettimeofday(&tval_before_custom, NULL);

        for(i=0;i<num_of_mallocs;i++) {
                pointer_main = (int *)malloc(sizeof(int));
                free(pointer_main);
        }

        gettimeofday(&tval_after_custom, NULL);

        timersub(&tval_after_custom, &tval_before_custom, &tval_result_custom);

        sec_standard = (long int)tval_result_standard.tv_sec;
        sec_custom = (long int)tval_result_custom.tv_sec;
        total_usec_standard = sec_standard * 10^6 + (long int)tval_result_standard.tv_usec;
        total_usec_custom = sec_custom * 10^6 + (long int)tval_result_custom.tv_usec;
        ratio = ((double)total_usec_custom) / ((double)total_usec_standard);
        ratio_sum += ratio;

        #ifdef time_measure_1_thread_print
            printf("Time elapsed with STANDARD malloc: %ld.%06ld\n", (long int)tval_result_standard.tv_sec, (long int)tval_result_standard.tv_usec);
            printf("Time elapsed with CUSTOM malloc:   %ld.%06ld\n", (long int)tval_result_custom.tv_sec, (long int)tval_result_custom.tv_usec);
            printf("total_usec_standard = %ld\n",total_usec_standard);
            printf("total_usec_custom = %ld\n",total_usec_custom);
            printf("Time difference: %06ld ms\n",(long int)tval_result_custom.tv_usec-(long int)tval_result_standard.tv_usec);
            printf("Time needed: %1.7f more\n\n",ratio);
        #endif

        __malloc_hook  = old_malloc_hook;
        __free_hook = old_free_hook;

    }

    printf("Iterations: %d, allocations and frees per iteration: %d\n",iterations,num_of_mallocs);
    printf("In average %1.7f more time was needed with custom malloc and free hooks.\n",ratio_sum/iterations);
#endif // time_measure

#ifdef modify_malloc_hook

    for(i=0;i<100;i++) {
        if(i%2 == 0) __malloc_hook = my_malloc_hook_1;
        else __malloc_hook = my_malloc_hook_2;

        pointer_main = (int *)malloc(sizeof(int));
        free(pointer_main);

    }


#endif // modify_malloc_hook


    getchar();
    if(hook_counter != 0) printf("hook_counter is not zero, %d\n",hook_counter);
    sem_destroy(&mutex);

    return 0;
}
