import java.util.concurrent.Semaphore;
import java.lang.InterruptedException;
import java.util.concurrent.TimeUnit;

class NatBoundedBuffer extends BoundedBuffer {

   // Initialise the protected buffer structure above.
   NatBoundedBuffer(int maxSize) {
      super(maxSize);
   }

   // Extract an element from buffer. If the attempted operation is
   // not possible immediately, the method call blocks until it is.
   synchronized Object get() {
      // Enter mutual exclusion done with synchronized -> taking the lock
      Object value;

      // Wait until there is a full slot available.
      while (size == 0) {
         try {
            wait();
         } catch (InterruptedException e) {
            e.printStackTrace();
         }
      }

      // Signal or broadcast that an empty slot is available (if needed)
      notifyAll();

      // Leave mutual exclusion done with synchronized -> releasing the lock
      return super.get();
   }

   // Insert an element into buffer. If the attempted operation is
   // not possible immedidately, the method call blocks until it is.
   synchronized boolean put(Object value) {
      // Enter mutual exclusion done with synchronized -> taking the lock

      // Wait until there is a empty slot available.
      while (size == maxSize) {
         try {
            wait();
         } catch (InterruptedException e) {
            e.printStackTrace();
         }
      }

      // Signal or broadcast that a full slot is available (if needed)
      notifyAll();

      super.put(value);

      // Leave mutual exclusion done with synchronized -> releasing the lock
      return true;
   }

   // Extract an element from buffer. If the attempted operation is not
   // possible immedidately, return NULL. Otherwise, return the element.
   synchronized Object remove() {
      // Enter mutual exclusion done with synchronized -> taking the lock

      // Check if a full slot is available
      if (size == 0) {
         // Leave mutual exclusion done with synchronized -> releasing the lock
         return null;
      } else {
         // Signal or broadcast that an empty slot is available (if needed)
         notifyAll();
         // Leave mutual exclusion done with synchronized -> releasing the lock
         return super.get();
      }
   }

   // Insert an element into buffer. If the attempted operation is
   // not possible immedidately, return 0. Otherwise, return 1.
   synchronized boolean add(Object value) {
      // Enter mutual exclusion done with synchronized -> taking the lock
      boolean done;

      // Check if a empty slot is available
      if (size == maxSize) {
         // Leave mutual exclusion done with synchronized -> releasing the lock
         return false;
      } else {
         // Signal or broadcast that a full slot is available (if needed)
         notifyAll();
         super.put(value);
         // Leave mutual exclusion done with synchronized -> releasing the lock
         return true;
      }
   }

   // Extract an element from buffer. If the attempted operation is not
   // possible immedidately, the method call blocks until it is, but
   // waits no longer than the given deadline. Return the element if
   // successful. Otherwise, return NULL.
   synchronized Object poll(long deadline) {
      // Enter mutual exclusion done with synchronized -> taking the lock
      long timeout;

      // Wait until a full slot is available but wait
      // no longer than the given deadline
      while (size == 0) {
         timeout = deadline - System.currentTimeMillis();
         if (timeout <= 0) {
            // Leave mutual exclusion done with synchronized -> releasing the lock
            return null;
         }
         try {
            wait(timeout);
         } catch (InterruptedException e) {
            e.printStackTrace();
         }
      }

      if (size == 0)
         return null;

      // Signal or broadcast that an full slot is available (if needed)
      notifyAll();

      // Leave mutual exclusion done with synchronized -> releasing the lock
      return super.get();
   }

   // Insert an element into buffer. If the attempted operation is not
   // possible immedidately, the method call blocks until it is, but
   // waits no longer than the given deadline. Return 0 if not
   // successful. Otherwise, return 1.
   synchronized boolean offer(Object value, long deadline) {
      // Enter mutual exclusion done with synchronized -> taking the lock
      long timeout;

      // Wait until a empty slot is available but wait
      // no longer than the given deadline
      while (size == maxSize) {
         timeout = deadline - System.currentTimeMillis();
         if (timeout <= 0) {
            // Leave mutual exclusion done with synchronized -> releasing the lock
            return false;
         }
         try {
            wait(timeout);
         } catch (InterruptedException e) {
            e.printStackTrace();
         }
      }

      if (size == maxSize)
         return false;

      // Signal or broadcast that an empty slot is available (if needed)
      notifyAll();

      super.put(value);

      // Leave mutual exclusion done with synchronized -> releasing the lock
      return true;
   }
}
