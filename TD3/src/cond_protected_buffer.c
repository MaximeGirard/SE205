
#include "circular_buffer.h"
#include "protected_buffer.h"
#include "utils.h"
#include <errno.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

// Initialise the protected buffer structure above.
protected_buffer_t *cond_protected_buffer_init(int length)
{
  protected_buffer_t *b;
  b = (protected_buffer_t *)malloc(sizeof(protected_buffer_t));
  b->buffer = circular_buffer_init(length);
  // Initialize the synchronization components
  pthread_mutex_init(&b->mut_exclusion, NULL);
  pthread_cond_init(&b->buffer_not_empty, NULL);
  pthread_cond_init(&b->buffer_not_full, NULL);
  return b;
}

// Extract an element from buffer. If the attempted operation is
// not possible immedidately, the method call blocks until it is.
void *cond_protected_buffer_get(protected_buffer_t *b)
{
  void *d;

  // Enter mutual exclusion
  pthread_mutex_lock(&b->mut_exclusion);

  // Wait until there is a full slot to get data from the unprotected
  // circular buffer (circular_buffer_get).
  while (circular_buffer_size(b->buffer) < 1)
    pthread_cond_wait(&b->buffer_not_empty, &b->mut_exclusion);

  d = circular_buffer_get(b->buffer);

  // Signal or broadcast that an empty slot is available in the
  // unprotected circular buffer (if needed)
  pthread_cond_broadcast(&b->buffer_not_full);

  if (d == NULL)
    mtxprintf(pb_debug, "get (B) - data=NULL\n");
  else
    mtxprintf(pb_debug, "get (B) - data=%d\n", *(int *)d);

  // Leave mutual exclusion
  pthread_mutex_unlock(&b->mut_exclusion);

  return d;
}

// Insert an element into buffer. If the attempted operation is
// not possible immedidately, the method call blocks until it is.
void cond_protected_buffer_put(protected_buffer_t *b, void *d)
{

  // Enter mutual exclusion
  pthread_mutex_lock(&b->mut_exclusion);

  // Wait until there is an empty slot to put data in the unprotected
  // circular buffer (circular_buffer_put).
  while (!(circular_buffer_size(b->buffer) < b->buffer->max_size))
    pthread_cond_wait(&b->buffer_not_full, &b->mut_exclusion);

  circular_buffer_put(b->buffer, d);

  // Signal or broadcast that a full slot is available in the
  // unprotected circular buffer (if needed)
  pthread_cond_broadcast(&b->buffer_not_empty);

  if (d == NULL)
    mtxprintf(pb_debug, "put (B) - data=NULL\n");
  else
    mtxprintf(pb_debug, "put (B) - data=%d\n", *(int *)d);

  // Leave mutual exclusion
  pthread_mutex_unlock(&b->mut_exclusion);
}

// Extract an element from buffer. If the attempted operation is not
// possible immedidately, return NULL. Otherwise, return the element.
void *cond_protected_buffer_remove(protected_buffer_t *b)
{
  void *d;

  // Enter mutual exclusion
  pthread_mutex_lock(&b->mut_exclusion);

  if (circular_buffer_size(b->buffer) < 1)
  {
    d = NULL;
  }
  else
  {
    d = circular_buffer_get(b->buffer);
  }
  // Signal or broadcast that an empty slot is available in the
  // unprotected circular buffer (if needed)
  if (d != NULL)
    pthread_cond_broadcast(&b->buffer_not_full);

  if (d == NULL)
    mtxprintf(pb_debug, "remove (U) - data=NULL\n");
  else
    mtxprintf(pb_debug, "remove (U) - data=%d\n", *(int *)d);

  // Leave mutual exclusion
  pthread_mutex_unlock(&b->mut_exclusion);

  return d;
}

// Insert an element into buffer. If the attempted operation is
// not possible immedidately, return 0. Otherwise, return 1.
int cond_protected_buffer_add(protected_buffer_t *b, void *d)
{
  int done;

  // Enter mutual exclusion
  pthread_mutex_lock(&b->mut_exclusion);

  if (circular_buffer_size(b->buffer) < b->buffer->max_size)
  {
    done = circular_buffer_put(b->buffer, d);
    // Signal or broadcast that a full slot is available in the
    // unprotected circular buffer (if needed)
    pthread_cond_broadcast(&b->buffer_not_empty);
  }
  else
  {
    done = 0;
  }

  if (!done)
    d = NULL;

  if (d == NULL)
    mtxprintf(pb_debug, "add (U) - data=NULL\n");
  else
    mtxprintf(pb_debug, "add (U) - data=%d\n", *(int *)d);

  // Leave mutual exclusion
  pthread_mutex_unlock(&b->mut_exclusion);

  return done;
}

// Extract an element from buffer. If the attempted operation is not
// possible immedidately, the method call blocks until it is, but
// waits no longer than the given timeout. Return the element if
// successful. Otherwise, return NULL.
void *cond_protected_buffer_poll(protected_buffer_t *b,
                                 struct timespec *abstime)
{
  void *d = NULL;
  int rc = 0;

  // Enter mutual exclusion
  pthread_mutex_lock(&b->mut_exclusion);

  // Wait until there is a full slot to get data from the unprotected
  // circular buffer  (circular_buffer_get) but waits no longer than
  // the given timeout.
  while (circular_buffer_size(b->buffer) < 1)
  {
    rc = pthread_cond_timedwait(&b->buffer_not_empty, &b->mut_exclusion, abstime);
    if (rc == ETIMEDOUT)
    {
      mtxprintf(pb_debug, "poll (T) - data=NULL\n");
      // Leave mutual exclusion
      pthread_mutex_unlock(&b->mut_exclusion);
      return NULL;
    }
  }

  // Signal or broadcast that an empty slot is available in the
  // unprotected circular buffer (if needed)
  pthread_cond_broadcast(&b->buffer_not_full);
  d = circular_buffer_get(b->buffer);

  if (d == NULL)
    mtxprintf(pb_debug, "poll (T) - data=NULL\n");
  else
    mtxprintf(pb_debug, "poll (T) - data=%d\n", *(int *)d);

  // Leave mutual exclusion
  pthread_mutex_unlock(&b->mut_exclusion);

  return d;
}

// Insert an element into buffer. If the attempted operation is not
// possible immedidately, the method call blocks until it is, but
// waits no longer than the given timeout. Return 0 if not
// successful. Otherwise, return 1.
int cond_protected_buffer_offer(protected_buffer_t *b, void *d,
                                struct timespec *abstime)
{
  int rc = 0;
  int done = 0;

  // Enter mutual exclusion
  pthread_mutex_lock(&b->mut_exclusion);

  // Wait until there is an empty slot to put data in the unprotected
  // circular buffer (circular_buffer_put) but waits no longer than
  // the given timeout.
  while (!(circular_buffer_size(b->buffer) < b->buffer->max_size))
  {
    rc = pthread_cond_timedwait(&b->buffer_not_full, &b->mut_exclusion, abstime);
    if (rc == ETIMEDOUT)
    {
      mtxprintf(pb_debug, "offer (T) - data=NULL\n");
      // Leave mutual exclusion
      pthread_mutex_unlock(&b->mut_exclusion);
      return 0;
    }
  }

  // Signal or broadcast that a full slot is available in the
  // unprotected circular buffer (if needed)
  pthread_cond_broadcast(&b->buffer_not_empty);
  done = circular_buffer_put(b->buffer, d);
  if (!done)
    d = NULL;

  if (d == NULL)
    mtxprintf(pb_debug, "offer (T) - data=NULL\n");
  else
    mtxprintf(pb_debug, "offer (T) - data=%d\n", *(int *)d);

  // Leave mutual exclusion
  pthread_mutex_unlock(&b->mut_exclusion);

  return done;
}
