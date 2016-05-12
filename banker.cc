/********************************************************
 * banker.cc
 * Author: Regan McCooey
 * Date: 4/25/1
 * Assignment: banker's algorithm
 * Class: CSCI 346 Professor Walsh
 * Purpose: to implement Banker's algorithm to prevent
 * deadlock
 *******************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <semaphore.h>
#include "banker.h"
#include "scenarios.h"

pthread_mutex_t is_remain;
pthread_cond_t s;

struct p_resource {
   int allocated;
   int max;
};

struct thread_res {
   bool started;
   p_resource res[R];
};

struct node {
   int id;
   struct node *next;
   struct node *prev;
};

thread_res thread_list[N];
int remaining[R];
void setmax(int, int, int);
void starting(int, int, int);
void alloc(int, int, int);
void release(int, int, int);
void finished(int);
bool bankers();
void release_temp(int, int*, node*&, node*&);

int main(int argc, char **argv) {

   if (pthread_mutex_init(&is_remain, NULL)) {
      printf("Error: unable to initalize mutex\n");
      return -1;
   }
     
   if (pthread_cond_init(&s, NULL)) {
      printf("Error unable to initalize the semaphore\n");
      return -1;
   }

   for (int i = 0; i < R; i++) {
      remaining[i] = TOTAL[i];
   }
    pthread_t id[N];
    for (int i = 0; i < N; i++)
        pthread_create(&id[i], NULL, &scenarioA, NULL);
    
    for (int i = 0; i < N; i++)
        pthread_join(id[i], NULL);

    printf("All threads have finished... no deadlock!\n");
}

/***********************************************************
 * void setmax(int, int, int)
 * Pre: the calling thread is started and max, 
 * i and r are valid ints
 * Post: the max of that resource will be set 
 *********************************************************/

void setmax(int i, int r, int amt) {
   if (thread_list[i].started) {
      printf("Error: this thread has already set its max\n");
   } else if (amt > TOTAL[r]) {
      printf("Error: we don't physically have that amount of that resource\n");
   } else { 
      thread_list[i].res[r].max = amt;
   }
}

/***********************************************************
 * void starting(int)
 * Pre: i in a valid int, a max has been set
 * Post: the thread will be set as started
 * and all allocated values will be set to 0
 *********************************************************/
void starting(int i) {
   if (thread_list[i].started) {
      printf("ERROR: the thread has already started\n");
      return;
   }
   thread_list[i].started = true;
}

/***********************************************************
 * void alloc(int, int, int)
 * Pre: i, amt, and r are valid ints
 * Post: the amt will be allocated to thread i if 
 * the bankers algorithm returns true, meaning that it's 
 * safe 
 *********************************************************/
void alloc(int i, int r, int amt) {

   if (!thread_list[i].started) {
      printf("Error: this thread has not started\n");
      return;
   }

   if (thread_list[i].res[r].max < (amt + thread_list[i].res[r].allocated)) {
      printf("Error: can't allocate more than the max\n");
      return;
   }

   pthread_mutex_lock(&is_remain);
   bool done = false;
   while (!done) {
      printf("Thread %d is trying to allocate %d of resource %d\n", i, amt, r); 
      //if amt isn't avail wait 
      if (amt > remaining[r]) {
	 printf("resource is not available waiting\n");
	 pthread_cond_wait(&s, &is_remain);
	 printf("done waiting\n");
      } else {
	 bool safe = false;
	 remaining[r] -= amt;
	 thread_list[i].res[r].allocated += amt;
	 printf("testing if this allocation is safe\n");
	 safe = bankers();
	 if (!safe) {
	    printf("allocation is not safe, waiting\n");
	    remaining[r]+= amt;
	    thread_list[i].res[r].allocated -= amt;
	    pthread_cond_wait(&s, &is_remain);
	    printf("done waiting\n");
	 } else {
	    pthread_mutex_unlock(&is_remain);
	    done = true;
	    printf("allocation is safe, complete\n\n");
	 }
      }
   }

}

/***********************************************************
 * void release(int, int, int)
 * Pre: i, amt, and r are valid ints
 * Post: the amt will be released to the remaining array
 *********************************************************/

void release(int i, int r, int amt) {

   //more than one thread should not release at the same time
   if (amt > 0) {
      pthread_mutex_lock(&is_remain);
      thread_list[i].res[r].allocated -= amt;
      remaining[r] += amt;
      pthread_cond_broadcast(&s);
      pthread_mutex_unlock(&is_remain);
   }
}

/***********************************************************
 * void finished(int)
 * Pre: i is a valid int
 * Post: all of process i's resources will be returned to
 * the remaining array and the thread will exit
 *********************************************************/
void finished(int i) {

   for (int j = 0; j < R; j++) {
      release(i, j, thread_list[i].res[j].allocated);
   }
   thread_list[i].started = false;
   pthread_exit(NULL);

}

/***********************************************************
 * void bankers()
 * Pre: the maxes are all set for the threads that have 
 * been started
 * Post: true or false will be returned based on 
 * whether or not the threads can safely finish
 *********************************************************/

bool bankers() {

   node *current = NULL;
   node *head = NULL;
   int t_started = 0;
   int num_t = 0;
   for (int i = 0; i < N; i++) {
      if (thread_list[i].started) {
	 if (head == NULL) {
	    head = new node();
	    head->id = i;
	    current = head;
	    current->prev = NULL;
	    current->next = NULL;
	 } else {
	    current->next = new node();
	    current->next->prev = current;
	    current = current->next;
	    current->id = i;
	    current->next = NULL;
	 }
	 t_started++;
      }
   }
      
   num_t = t_started;

   int temp_remain[R];
   for (int i = 0; i < R; i++) {
      temp_remain[i] = remaining[i];
   }

   int exec_list[N];
   int p_count = 0;
   bool deleted = false;
   bool madeMoves = false;

   while (true) {
      node * c = head;
      madeMoves = false;
      //go back around again 
      //if you go through the whole thing and dont make any progress - thats when its bad 
      while(c != NULL) {
	 deleted = false;
	 for (int j = 0; j < R; j++) {
	    int ask = thread_list[c->id].res[j].max - thread_list[c->id].res[j].allocated;
	    if (ask > temp_remain[j]) {
	       break;
	    }
	    if (j == R-1) {
	       exec_list[p_count] = c->id;
	       p_count++;
	       release_temp(c->id, temp_remain, head, c);
	       deleted = true;
	       t_started--;
	       madeMoves = true;
	    }
	 }
	 if (!deleted) {
	    c = c->next;
	 }
      }
      if (head == NULL) {
	 printf("State is safe! processes could finish in this order:\n");
	 for (int i = 0; i < p_count; i++) {
	    printf("%d ", exec_list[i]);
	 }
	 printf("\n");
	 return true;
      }
      if (!madeMoves && c == NULL) {
	 printf("This state is unsafe\n");
	 return false;
      }
   }
}

/***********************************************************
 * void release_temp(int, int*, node*&, node*&)
 * Pre: i, amt, and r are valid ints, and current is 
 * the node we want to remove and release
 * Post: the node will be deleted from the doubly linked
 * list 
 *********************************************************/
void release_temp(int i, int *temp, node *&head, node *&current) {
   for (int j = 0; j < R; j++) {
      temp[j] += thread_list[i].res[j].allocated;
   }
   //what about when we delete this and then move current? 
   node *d = current;
   if (current == head) {
      d = head;
      head = head->next;
      if (head != NULL) {
	 head->prev = NULL;
	 d->next = NULL;
      }
      current = head;
      delete d;
   } else {
      node *p = current->prev;
      p->next = current->next;
      if (current->next != NULL) {
	 current->next->prev = p;
      }
      current = current->next;
      d->next = NULL;
      d->prev = NULL;
      delete d;
   }
}
