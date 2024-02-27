#ifndef MUTEX_H
#define MUTEX_H

typedef struct mutex
{
    volatile atomic_uint ticket;
    volatile unsigned int turn;
} mutex_t;

mutex_t init_mutex();
void lock(mutex_t *mutex);
void unlock(mutex_t *mutex);

#endif // MUTEX_H