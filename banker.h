// Banker's Algorithm Project
// kwalsh@cs.holycross.edu
#ifndef BANKER_H
#define BANKER_H

// This header file defines some constants for the resources.


// There are at most N=5 threads competing for resources.
#define N 5

// There are R=4 resources: keyboard, disk, memory, and network connections
#define R 4

// Each resource is assigned an integer ID.
#define KBD 0
#define DISK 1
#define MEM 2
#define NET 3

// Each resource has a name, for pretty printing, error messages, etc.
const char * const RNAME[] = { "keyboard", "disk space", "memory pages", "network connections" };

// Total amount of each resource in the system. This never changes.
const int TOTAL[] = { 1,  50000, 1000, 100 };


// The five functions below make up the Banker's algorithm. In each scenario for
// testing, up to five different threads will call all of these functions,
// repeatedly and possibly even simultaneously. All of the functions will need
// to use mutexes, semaphores, condition variables, etc., in order to ensure
// there are no race conditions (i.e. so the functions are "thread-safe").
//
// Each calling thread will typically follow the same sequence:
//   First call setmax() a few times, at most once for each resource.
//   Next call starting().
//   Next call alloc() and/or release(), repeatedly, as many times as desired.
//   Lastly, call finished().

// By calling setmax(), thread _i_ is promising that it will never try to
// allocate more than _amt_ of resource _r_. The thread ID _i_ will be 0 <= i <
// N, and the resource number _r_ will be 0 <= r < R. That is, _r_ is one of the
// constants: KBD, DISK, MEM, or NET. 
//
// * It is an error for a thread to call this after it has called starting().
// * It is an error for a thread to call this with amt > TOTAL[r].
void setmax(int i, int r, int amt);

// Function starting() will be called by thread _i_ after it done calling
// setmax() and before it calls any of the other functions below.
//
// * It is an error for a thread to call this after it has previously called
//   starting().
void starting(int i);

// Thread _i_ calls alloc() to allocate amount _amt_ of resource _r_.
// The banker's algorithm should first check if this allocation could
// potentially lead to future deadlocks. If it is safe, the function should go
// ahead and make (well, fake, actually) the allocation, updating the total
// amount in the banker's algorithm tables. Otherwise, if it is unsafe, the
// function should wait and try again later when it may be safe. Assuming the
// banker's algorithm is followed and all other threads behave (i.e. eventually
// call finish() to release their resources), the alloc() function will
// eventually succeed.
//
// * It is an error for a thread to call this before it has called starting().
// * It is an error for a thread to call this unless setmax() was called
//   previously.
// * It is an error for _amt_, plus any previous amounts of this resource still
//   allocated by this thread, to exceed the maximum specified when the thread
//   called setmax().
void alloc(int i, int r, int amt);

// Thread _i_ calls release() to relinquish _amt_ of resource _r_. The bankers
// algorithm should update its bookkeeping as needed, perhaps also waking up
// threads that are waiting for resources.
//
// * It is an error for a thread to call this before it has called starting().
// * It is an error for a thread to call this unless setmax() was called
//   previously.
// * It is an error for _amt_ to exceed the total previous amounts of this
//   resource allocated by this thread.
void release(int i, int r, int amt);

// Thread _i_ calls finished() to exit and relinquish any remaining resources it
// still holds. The banker's algorithm should update its bookkeeping as needed,
// just as if release() was called for any resources this thread still holds.
// Then, the function will cause the current thread to exit by calling
// pthread_exit(NULL). This ensures the thread is dead and none of the other
// funtions will be called again by this thread.
void finished(int i);

#endif // BANKER_H
