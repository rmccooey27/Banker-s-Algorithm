// Banker's Algorithm Project
// kwalsh@cs.holycross.edu
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include "banker.h"
#include "scenarios.h"

// This file implements four scenarios for testing banker's algorithm.
// See scenarios.h for how to use these scenarios.

// Note: The code here has a nice example of how to use pthread mutexes. Further
// down below is a second example, using mutexes and condition variables.

// In all of the scenarios below, we need to assign each thread an ID, an
// integer between 0 and N. We keep a global variable called "next_id" that gets
// incremented by each thread when it calls the getid() function. But notice, if
// we aren't careful about synchronization, this would be a race condition! To
// assign the IDs in a thread-safe manner requires using a mutex, to ensure that
// no two threads simultanously choose the same ID.

int next_id = 0; // protected by id_lock mutex
pthread_mutex_t id_lock = PTHREAD_MUTEX_INITIALIZER; // protects next_id variable

// This function returns a fresh, unique number to be used as the ID for the
// calling thread.
int getid() {
    pthread_mutex_lock(&id_lock); // start of critical section
    if (next_id >= N) {
        printf("error: can't start more than %d threads.\n", N);
        // note: in error cases like this, we must be careful to unlock the
        // mutex, otherwise subsequent calls to this function would be
        // deadlocked.
        pthread_mutex_unlock(&id_lock); // end critical section early due to error
        pthread_exit(NULL); // quit current thread
    }
    int my_id = next_id;
    next_id++;
    pthread_mutex_unlock(&id_lock); // end of critical section
    return my_id;
}

// Scenario A: This is taken almost directly from the paper assignment.
//
//     total resources
//     kbd   disk   mem  net
//       1  50000  1000  100
//
//     current allocation        maximum requested
//     kbd   disk   mem  net     kbd   disk   mem  net
// P0    0  20000   300   50       0  40000   500   90
// P1    0      0    50    0       1  10000   150   10
// P2    1  10000   150   10       1  15000   150   10
// P3    0   5000   100    0       0  30000   150    0
// P4    0  10000   400    0       1  10000   600   10
//
//     left remaining
//     kbd   disk   mem  net
//       0   5000     0   40

// Scenario A involves two phases:
//   phase 1: Set up the maximum resource demands (as shown in the "maximum
//     requested table above"), and making some initial allocations (as shown in
//     the "current allocation table" above)
//   phase 2: Make further allocations, up to the maximum, then quit the thread.
// At the end of phase 1, there is a "pause", or "barrier", or a "rendezvous".
// Once all five threads reach the rendezvous, then we are in exactly the
// scenario described by the paper assignment.
// Phase 2 then goes on to make the rest of the allocations. If all goes well,
// the threads should finish in the same order you found when you ran the
// banker's algorithm on paper. Actually, there may be more than one valid
// ordering, but you probably noticed that when you did it on paper.

// These variables are used by scenarioA to coordinate the "barrier" between the
// phases.
int rendezvous_reached = 0; // how many threads have finished phase 1, protected by rendezvous_lock
pthread_mutex_t rendezvous_lock = PTHREAD_MUTEX_INITIALIZER; // protects rendezvous_reached
pthread_cond_t rendezvous_cond = PTHREAD_COND_INITIALIZER; // tracks changes to rendezvous_reached

void *scenarioA(void *ignored) {
    int my_id = getid();

    // Phase 1
    switch (my_id) {
        case 0:
            setmax(my_id, KBD, 0);
            setmax(my_id, DISK, 40000);
            setmax(my_id, MEM, 500);
            setmax(my_id, NET, 90);
            starting(my_id);
            alloc(my_id, DISK, 20000);
            alloc(my_id, MEM, 300);
            alloc(my_id, NET, 50);
            break;
        case 1:
            setmax(my_id, KBD, 1);
            setmax(my_id, DISK, 10000);
            setmax(my_id, MEM, 150);
            setmax(my_id, NET, 10);
            starting(my_id);
            alloc(my_id, MEM, 50);
            break;
        case 2:
            setmax(my_id, KBD, 1);
            setmax(my_id, DISK, 15000);
            setmax(my_id, MEM, 150);
            setmax(my_id, NET, 10);
            starting(my_id);
            alloc(my_id, KBD, 1);
            alloc(my_id, DISK, 10000);
            alloc(my_id, MEM, 150);
            alloc(my_id, NET, 10);
            break;
        case 3:
            setmax(my_id, KBD, 0);
            setmax(my_id, DISK, 30000);
            setmax(my_id, MEM, 150);
            setmax(my_id, NET, 0);
            starting(my_id);
            alloc(my_id, DISK, 5000);
            alloc(my_id, MEM, 100);
            break;
        case 4:
            setmax(my_id, KBD, 1);
            setmax(my_id, DISK, 10000);
            setmax(my_id, MEM, 600);
            setmax(my_id, NET, 10);
            starting(my_id);
            alloc(my_id, DISK, 10000);
            alloc(my_id, MEM, 400);
            break;
    }

    // Pause:
    // Wait here until all five threads have allocated their resources. If all
    // threads get to here, then the banker's algorithm must have decided that
    // the system is safe.
    pthread_mutex_lock(&rendezvous_lock);
    printf("Thread %d is waiting for siblings to catch up...\n", my_id);
    rendezvous_reached++;
    while (rendezvous_reached != 5) {
        pthread_cond_wait(&rendezvous_cond, &rendezvous_lock);
    }
    pthread_cond_broadcast(&rendezvous_cond);
    pthread_mutex_unlock(&rendezvous_lock);
    printf("Hurray, no deadlock! Thread %d is continuing!\n", my_id);

    // Phase 2
    switch (my_id) {
        case 0:
            alloc(my_id, MEM, 200);
            alloc(my_id, DISK, 5000);
            alloc(my_id, NET, 40);
            alloc(my_id, DISK, 15000);
            break;
        case 1:
            alloc(my_id, DISK, 2000);
            alloc(my_id, KBD, 1);
            alloc(my_id, MEM, 50);
            alloc(my_id, NET, 10);
            alloc(my_id, DISK, 8000);
            alloc(my_id, MEM, 50);
            break;
        case 2:
            alloc(my_id, DISK, 5000);
            break;
        case 3:
            alloc(my_id, DISK, 20000);
            alloc(my_id, MEM, 50);
            alloc(my_id, DISK, 5000);
            break;
        case 4:
            alloc(my_id, MEM, 100);
            alloc(my_id, NET, 3);
            alloc(my_id, MEM, 100);
            alloc(my_id, NET, 7);
            alloc(my_id, KBD, 1);
            break;
    }

    printf("Thread %d signing off!\n", my_id);
    finished(my_id);
    return NULL;
}

// Scenario B: A simple scenario.
// Each thread declares some maximum resources, then does a short sequence of
// alloc(), sleep(), and release() calls. There is no particular rhyme or reason
// to the numbers chosen here.

void *scenarioB(void *ignored) {
    int my_id = getid();

    if (my_id == 0) {
        // Thread 0 wants kbd, some memory, and some network.
        setmax(my_id, KBD, 1);
        setmax(my_id, MEM, 500);
        setmax(my_id, NET, 30);
        // It allocates a few things, releases some things, then quits.
        starting(my_id);
        alloc(my_id, KBD, 1);
        alloc(my_id, MEM, 200);
        alloc(my_id, NET, 20);
        sleep(1);
        release(my_id, NET, 10);
        release(my_id, KBD, 1);
        alloc(my_id, MEM, 300);
        sleep(1);
    } else if (my_id == 1 || my_id == 2) {
        // Threads 1 and 2 want kbd, a little memory, and lots of disk.
        setmax(my_id, KBD, 1);
        setmax(my_id, MEM, 100);
        setmax(my_id, DISK, 35000);
        // Each allocates a few things, releases some things, then quits.
        starting(my_id);
        alloc(my_id, MEM, 50);
        alloc(my_id, DISK, 20000);
        alloc(my_id, KBD, 1);
        alloc(my_id, MEM, 50);
        sleep(1);
        release(my_id, KBD, 1);
        release(my_id, MEM, 50);
        alloc(my_id, DISK, 15000);
        sleep(1);
    } else if (my_id == 3 || my_id == 4) {
        // Threads 1 and 2 want some memory, some disk, and some network.
        setmax(my_id, MEM, 200);
        setmax(my_id, DISK, 20000);
        setmax(my_id, NET, 50);
        // Each allocates a few things, releases some things, then quits.
        starting(my_id);
        alloc(my_id, MEM, 100);
        alloc(my_id, DISK, 10000);
        alloc(my_id, NET, 25);
        sleep(1);
        alloc(my_id, DISK, 10000);
        alloc(my_id, MEM, 50);
        release(my_id, NET, 25);
        sleep(1);
        release(my_id, DISK, 20000);
        alloc(my_id, NET, 50);
        release(my_id, MEM, 25);
    }

    printf("Thread %d all done!\n", my_id); 
    finished(my_id);
    return NULL;
}

// Scenario C: A moderate length randomized scenario.
// Thread 0, 1, and 2 need lots of memory, but not much disk or network.
// Thread 4 and 5 need lots of network and disk, but only a little memory.
// Threads 0 and 4 need the keyboard.
// Each thread does three rounds of alloc, sleep, and release before finishing.
// The amounts for each alloc() and release() are chosen randomly.
//
// Note: The standard random number generators, e.g. rand(), are not
// thread-safe. It can crash if you call it simultaneously from multiple
// threads. Here we rand_r(), the "thread-safe" version of rand(). For ease of
// debugging, each thread uses a seed equal to it's ID, which causes each thread
// to replay the same sequence of alloc() and release() calls every time you run
// it. You can change the seed to anything you like to get different scenarios.
// Even with the same seed, however, timing differences due to the scheduler
// will affect the outcome. That's inevitable with any concurrent multi-threaded
// program.

void testC(int my_id, int kbd, int disk, int mem, int net) {
    unsigned int seed = (unsigned)my_id; // seed for random number generator, can be anything

    // This code could be a lot shorter if we used arrays to hold the variables,
    // but I wanted to keep the code simpler (even if longer) instead, to be
    // sure the purpose of the code is clear.
    setmax(my_id, KBD, kbd);
    setmax(my_id, DISK, disk);
    setmax(my_id, MEM, mem);
    setmax(my_id, NET, net);

    starting(my_id);

    // These variables keep track of how much we allocated so far.
    int k = 0, d = 0, m = 0, n = 0;

    for (int count = 0; count < 3; count++) {

        // Allocate a random amount of each resource.
        if (k != kbd) {
            int amt = rand_r(&seed) % (kbd - k);
            k += amt;
            if (amt > 0) alloc(my_id, KBD, amt);
        }
        if (d != disk) {
            int amt = rand_r(&seed) % (disk - d);
            d += amt;
            if (amt > 0) alloc(my_id, DISK, amt);
        }
        if (m != mem) {
            int amt = rand_r(&seed) % (mem - m);
            m += amt;
            if (amt > 0) alloc(my_id, MEM, amt);
        }
        if (n != net) {
            int amt = rand_r(&seed) % (net - n);
            n += amt;
            if (amt > 0) alloc(my_id, NET, amt);
        }

        // Sleep a little, either 1 second or half a second.
        if ((rand_r(&seed) % 2) == 0) usleep(1000000);
        else usleep(500000);

        // Release a random amount of each resource.
        if (k > 0) {
            int amt = rand_r(&seed) % k;
            k -= amt;
            if (amt > 0) release(my_id, KBD, amt);
        }
        if (d > 0) {
            int amt = rand_r(&seed) % d;
            d -= amt;
            if (amt > 0) release(my_id, DISK, amt);
        }
        if (m > 0) {
            int amt = rand_r(&seed) % m;
            m -= amt;
            if (amt > 0) release(my_id, MEM, amt);
        }
        if (n > 0) {
            int amt = rand_r(&seed) % n;
            n -= amt;
            if (amt > 0) release(my_id, NET, amt);
        }

    }

    finished(my_id);
}

void *scenarioC(void *ignored) {
    int my_id = getid();
    switch (my_id) {
        //                 kbd   disk  mem net
        case 0: testC(my_id, 1,  1000, 500,  5); break;
        case 1: testC(my_id, 0,  1000, 800, 10); break;
        case 2: testC(my_id, 0,  2000, 600,  0); break;
        case 3: testC(my_id, 0, 45000, 200, 75); break;
        case 4: testC(my_id, 1, 30000, 200, 60); break;
    }
    return NULL;
}

// Scenario D: A longer randomized scenario.
// Every thread declares a random max amount for each resource. Every thread
// then randomly allocates or releases a random amount of a random resource,
// repeatedly for a while. Small sleeps are inserted between to keep things
// interesting.
//
// As described above, here we again use the thread ID as a seed for the random
// number generator.

void *scenarioD(void *ignored) {
    int my_id = getid();

    unsigned int seed = (unsigned)my_id; // seed for random number generator, can be anything

    // These track of how much we want, and how much we want, of each resource.
    int want[R], have[R];

    for (int r = 0; r < R; r++) {
        have[r] = 0;
        want[r] = rand_r(&seed) % TOTAL[r];
        printf("Thread %d will want up to %d of %s.\n", my_id, want[r], RNAME[r]);
        setmax(my_id, r, want[r]);
    }

    starting(my_id);

    for (int count = 0; count < 5000; count++) {
        int r = rand_r(&seed) % R;
        if (want[r] == 0)
            continue;
        if (have[r] == 0 || (rand_r(&seed) % 2) == 0) {
            int amt = rand_r(&seed) % (want[r] - have[r]);
            if (amt > 0)
                alloc(my_id, r, amt);
            have[r] += amt;
        } else {
            int amt = rand_r(&seed) % have[r];
            if (amt > 0)
                release(my_id, r, amt);
            have[r] -= amt;
        }
        usleep(rand_r(&seed) % 1000);
    }

    finished(my_id);
    return NULL;
}

