// Banker's Algorithm Project
// kwalsh@cs.holycross.edu
#ifndef SCENARIOS_H
#define SCENARIOS_H

// This header file defines four scenarios for testing banker's algorithm.

// The idea is that main() should call:
//     pthread_t id;
//     pthread_create(&id, NULL, &scenarioX, NULL);
// five times, to start five independent threads executing that scenario. You
// will need five separate id variables, or use an array of them.
// Then, main should call:
//     pthread_join(&id, NULL);
// five times, once for each of your id variables.
//
// For scenarioA, there must be five threads, and all five threads must be
// executing the same scenario. For the others, you can use anywhere from 1 to 5
// threads, and you can mix-and-match the scenarios if you like, e.g. two thread
// executing scenarioB, a thread executing scenarioC, etc.

void *scenarioA(void *ignored); // The same scenario as the paper assignment.
void *scenarioB(void *ignored); // A similar, simple scenario.
void *scenarioC(void *ignored); // Moderate length random scenario.
void *scenarioD(void *ignored); // Longer random scenario.

#endif // SCENARIOS_H
