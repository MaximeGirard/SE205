#include <stdio.h>
#include <pthread.h>

void *myThread1(void *parameter)
{
    printf("Hello World 1 !\n");
    return NULL;
}

void *myThread2(void *parameter)
{
    printf("Hello World 2 !\n");
    return NULL;
}

int main()
{
    pthread_t thread1; // create and start thread 1
    pthread_t thread2; // create and start thread 2

    pthread_create(&thread1, NULL, myThread1, NULL);
    pthread_create(&thread2, NULL, myThread2, NULL);
    pthread_join(thread1, NULL); // wait for thread to terminate
    pthread_join(thread2, NULL); // wait for thread to terminate

    return 0;
}
