#include <stdio.h>
#include <stdatomic.h>
#include "mutex.h"

mutex_t init_mutex()
{
    printf("Mutex initialised\n");
    return (mutex_t){0, 0};
}

void lock(mutex_t *mutex) {
    int local_ticket = atomic_fetch_add(&mutex->ticket, 1);
    while (local_ticket != mutex->turn){};
    return;
}

void unlock(mutex_t *mutex) {
    mutex->turn++;
    atomic_thread_fence(memory_order_seq_cst);
}