#ifndef GLOBALS_H
#define GLOBALS_H

#include <semaphore.h>
#include <pthread.h>

// ----------------------------
// Global concurrency control
// ----------------------------

// Maximum number of concurrent worker threads/sockets
constexpr int MAX_CONCURRENCY = 300;

// Declare the semaphore as extern so it can be used across files
extern sem_t connection_sem;

// ----------------------------
// Global mutexes (if you use them for logging)
// ----------------------------
//extern pthread_mutex_t the_mutex1;
//extern pthread_mutex_t the_mutex2;
//xtern pthread_mutex_t the_mutex3;

#endif // GLOBALS_H
