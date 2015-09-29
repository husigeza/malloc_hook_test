#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>
#include <sys/time.h>
#include <semaphore.h>

//#define time_measure
#define thread_safe
//#define modify_malloc_hook


/* Prototypes for our hooks.  */
static void *old_malloc_hook;
static void *my_malloc_hook_0 (size_t, const void *);
static void *my_malloc_hook_1 (size_t, const void *);
static void *my_malloc_hook_2 (size_t, const void *);


static void *old_free_hook;
static void *my_free_hook_0 (void *, const void *);



pthread_t tid[3];

void* Thread_0(void *arg)
{
    int i,j=0;
    int *pointer_0;

    int argument = (int)arg;


    for(i=0;i<100;i++)
    {
        __malloc_hook = my_malloc_hook_0;
        printf("Thread %d, malloc %d\n",argument,i);
        pointer_0 = (int *)malloc(sizeof(int));
        free(pointer_0);
        usleep(500);
    }

    return NULL;

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

//void (*volatile __malloc_initialize_hook) (void) = my_init_hook;

static int malloc_hook_counter = 0;
static sem_t mutex;

static void *
my_malloc_hook_0 (size_t size, const void *caller)
{
      //printf("Malloc hook 0\n");
      __malloc_hook  = old_malloc_hook;
      malloc(size);
      __malloc_hook = my_malloc_hook_0;

    sem_wait(&mutex);
      malloc_hook_counter++;
      printf("malloc_hook_counter = %d\n",malloc_hook_counter);
    sem_post(&mutex);

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


#ifdef thread_safe

    sem_init(&mutex, 0, 1);

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

    getchar();
    sem_destroy(&mutex);
#endif // thread_safe

#ifdef time_measure

    for(j=0;j<10;j++){

        gettimeofday(&tval_before_standard, NULL);

        for(i=0;i<100000;i++) {
                pointer_main = (int *)malloc(sizeof(int));
                free(pointer_main);
        }

        gettimeofday(&tval_after_standard, NULL);

        timersub(&tval_after_standard, &tval_before_standard, &tval_result_standard);

        printf("Time elapsed with STANDARD malloc: %ld.%06ld\n", (long int)tval_result_standard.tv_sec, (long int)tval_result_standard.tv_usec);


        __malloc_hook = my_malloc_hook_0;
        __free_hook = my_free_hook_0;
        gettimeofday(&tval_before_custom, NULL);

        for(i=0;i<100000;i++) {
                pointer_main = (int *)malloc(sizeof(int));
                free(pointer_main);
        }

        gettimeofday(&tval_after_custom, NULL);

        timersub(&tval_after_custom, &tval_before_custom, &tval_result_custom);

        printf("Time elapsed with CUSTOM malloc:   %ld.%06ld\n", (long int)tval_result_custom.tv_sec, (long int)tval_result_custom.tv_usec);

        long int sec_standard = (long int)tval_result_standard.tv_sec;
        long int total_usec_standard = sec_standard * 10^6 + (long int)tval_result_standard.tv_usec;

        long int sec_custom = (long int)tval_result_custom.tv_sec;
        long int total_usec_custom = sec_custom * 10^6 + (long int)tval_result_custom.tv_usec;

        printf("total_usec_standard = %ld\n",total_usec_standard);
        printf("total_usec_custom = %ld\n",total_usec_custom);
        double ratio = ((double)total_usec_custom) / ((double)total_usec_standard);

        printf("Time difference: %06ld ms\n",(long int)tval_result_custom.tv_usec-(long int)tval_result_standard.tv_usec);
        printf("Time needed: %1.7f more\n\n",ratio);


        __malloc_hook  = old_malloc_hook;

    }

#endif // time_measure

#ifdef modify_malloc_hook

    for(i=0;i<100;i++) {
        if(i%2 == 0) __malloc_hook = my_malloc_hook_1;
        else __malloc_hook = my_malloc_hook_2;

        pointer_main = (int *)malloc(sizeof(int));
        free(pointer_main);

    }


#endif // modify_malloc_hook


    return 0;
}
