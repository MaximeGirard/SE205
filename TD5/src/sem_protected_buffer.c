#include "circular_buffer.h"
#include "protected_buffer.h"
#include "utils.h"
#include <errno.h>
#include <fcntl.h>
#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>

#define EMPTY_SLOTS_NAME "/empty_slots"
#define FULL_SLOTS_NAME "/full_slots"

// Initialise the protected buffer structure above.
protected_buffer_t *sem_protected_buffer_init(int length)
{
  protected_buffer_t *b;
  b = (protected_buffer_t *)malloc(sizeof(protected_buffer_t));
  b->buffer = circular_buffer_init(length);
  // Initialize the synchronization attributes
  // Use these filenames as named semaphores
  sem_unlink(EMPTY_SLOTS_NAME);
  sem_unlink(FULL_SLOTS_NAME);
  // Open the semaphores using the filenames above
  b->buffer_empty_slot_count = sem_open(EMPTY_SLOTS_NAME, O_CREAT, 0644, 0);
  b->buffer_full_slot_count = sem_open(FULL_SLOTS_NAME, O_CREAT, 0644, length);
  return b;
}

// Extract an element from buffer. If the attempted operation is
// not possible immedidately, the method call blocks until it is.
void *sem_protected_buffer_get(protected_buffer_t *b)
{
  void *d;

  // Enforce synchronisation semantics using semaphores.
  sem_wait(b->buffer_empty_slot_count);

  // Enter mutual exclusion.
  pthread_mutex_lock(&b->mut_exclusion);

  d = circular_buffer_get(b->buffer);
  if (d == NULL)
    mtxprintf(pb_debug, "get (B) - data=NULL\n");
  else
    mtxprintf(pb_debug, "get (B) - data=%d\n", *(int *)d);

  // Leave mutual exclusion.
  pthread_mutex_unlock(&b->mut_exclusion);

  // Enforce synchronisation semantics using semaphores.
  sem_post(b->buffer_full_slot_count);

  return d;
}

// Insert an element into buffer. If the attempted operation is
// not possible immedidately, the method call blocks until it is.
void sem_protected_buffer_put(protected_buffer_t *b, void *d)
{

  // Enforce synchronisation semantics using semaphores.
  sem_wait(b->buffer_full_slot_count);

  // Enter mutual exclusion.
  pthread_mutex_lock(&b->mut_exclusion);

  circular_buffer_put(b->buffer, d);
  if (d == NULL)
    mtxprintf(pb_debug, "put (B) - data=NULL\n");
  else
    mtxprintf(pb_debug, "put (B) - data=%d\n", *(int *)d);

  // Leave mutual exclusion.
  pthread_mutex_unlock(&b->mut_exclusion);

  // Enforce synchronisation semantics using semaphores.
  sem_post(b->buffer_empty_slot_count);
}

// Extract an element from buffer. If the attempted operation is not
// possible immedidately, return NULL. Otherwise, return the element.
void *sem_protected_buffer_remove(protected_buffer_t *b)
{
  void *d = NULL;
  int rc = -1;

  // Enforce synchronisation semantics using semaphores.
  rc = sem_trywait(b->buffer_empty_slot_count);

  if (rc == 0)
  {
    // Enter mutual exclusion.
    pthread_mutex_lock(&b->mut_exclusion);

    d = circular_buffer_get(b->buffer);
    if (d == NULL)
      mtxprintf(pb_debug, "remove (U) - data=NULL\n");
    else
      mtxprintf(pb_debug, "remove (U) - data=%d\n", *(int *)d);

    // Leave mutual exclusion.
    pthread_mutex_unlock(&b->mut_exclusion);

    // Enforce synchronisation semantics using semaphores.
    sem_post(b->buffer_full_slot_count);

    return d;
  }
  else
  {
    // We didn't successfuly locked the semaphore
    mtxprintf(pb_debug, "remove (U) - data=NULL\n");
    return NULL;
  }
}

// Insert an element into buffer. If the attempted operation is
// not possible immedidately, return 0. Otherwise, return 1.
int sem_protected_buffer_add(protected_buffer_t *b, void *d)
{
  int rc = -1;

  // Enforce synchronisation semantics using semaphores.
  rc = sem_trywait(b->buffer_full_slot_count);

  if (rc == 0)
  {
    // Enter mutual exclusion.
    pthread_mutex_lock(&b->mut_exclusion);

    circular_buffer_put(b->buffer, d);
    if (d == NULL)
      mtxprintf(pb_debug, "add (U) - data=NULL\n");
    else
      mtxprintf(pb_debug, "add (U) - data=%d\n", *(int *)d);

    // Leave mutual exclusion.
    pthread_mutex_unlock(&b->mut_exclusion);
    // Enforce synchronisation semantics using semaphores.
    sem_post(b->buffer_empty_slot_count);

    return 1;
  }
  else
  {
    // We didn't successfuly locked the semaphore
    mtxprintf(pb_debug, "add (U) - data=NULL\n");
    return 0;
  }
}

// Extract an element from buffer. If the attempted operation is not
// possible immedidately, the method call blocks until it is, but
// waits no longer than the given timeout. Return the element if
// successful. Otherwise, return NULL.
void *sem_protected_buffer_poll(protected_buffer_t *b,
                                struct timespec *abstime)
{
  void *d = NULL;
  int rc = -1;

  // Enforce synchronisation semantics using semaphores.
  rc = sem_timedwait(b->buffer_empty_slot_count, abstime);

  if (rc == 0)
  {
    // Enter mutual exclusion.
    pthread_mutex_lock(&b->mut_exclusion);

    d = circular_buffer_get(b->buffer);
    if (d == NULL)
      mtxprintf(pb_debug, "poll (T) - data=NULL\n");
    else
      mtxprintf(pb_debug, "poll (T) - data=%d\n", *(int *)d);

    // Leave mutual exclusion.
    pthread_mutex_unlock(&b->mut_exclusion);

    // Enforce synchronisation semantics using semaphores.
    sem_post(b->buffer_full_slot_count);

    return d;
  }
  else
  {
    // We didn't get the lock in time
    mtxprintf(pb_debug, "poll (T) - data=NULL\n");
    return NULL;
  }
}

// Insert an element into buffer. If the attempted operation is not
// possible immedidately, the method call blocks until it is, but
// waits no longer than the given timeout. Return 0 if not
// successful. Otherwise, return 1.
int sem_protected_buffer_offer(protected_buffer_t *b, void *d,
                               struct timespec *abstime)
{
  int rc = -1;

  // Enforce synchronisation semantics using semaphores.
  rc = sem_timedwait(b->buffer_full_slot_count, abstime);

  if (rc == 0)
  {
    // Enter mutual exclusion.
    pthread_mutex_lock(&b->mut_exclusion);

    circular_buffer_put(b->buffer, d);
    if (d == NULL)
      mtxprintf(pb_debug, "offer (T) - data=NULL\n");
    else
      mtxprintf(pb_debug, "offer (T) - data=%d\n", *(int *)d);

    // Leave mutual exclusion.
    pthread_mutex_unlock(&b->mut_exclusion);

    // Enforce synchronisation semantics using semaphores.
    sem_post(b->buffer_empty_slot_count);
    
    return 1;
  }
  else
  {
    // We didn't get the lock in time
    mtxprintf(pb_debug, "offer (T) - data=NULL\n");
    return 0;
  }
}
