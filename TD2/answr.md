# TD 2 - report

## 2 Array with two Producers

*Is the program race-free? Explain your answer in detail.*

The program is race-free. Because the counter is incremented atomicly, the two producers can't write on the same address (the two other conditions for a race are reunited).

*Which kind of initerleavings are possible for this program? Which interleavings can you observe? What might be happening here?*

The interleavings that are possible are the following:
- Producer 1 writes to the array continuously and never lets producer 2 write to the array
- Producer 1 writes to the array, then producer 2 writes to the array (and vice versa)

It's because in the first case, the second thread isn't created yet (it takes some time to create a thread).

## 3 Total Store Order

It ends but after several minutes.
Using the atomic_thread_fence, the program never ends.