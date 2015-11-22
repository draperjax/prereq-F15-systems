// Simulation of a Uber dispatch mechanism
// This is the only file where you need to write your own code!
// Your solution should not rely on modification to any other files!
// See dispatch.h for descriptions of the following functions
#include "dispatch.h"
#include <sys/time.h>

/**
 ** -- EXPLANATION --
 ** My strategy leverages mutexes with two conditionval variables, 
 ** one to indicate that there are requests coming in and one to indicate
 ** that drivers are done. The first CV is used (and waited on) by the drivers
 ** when they detect an empty queue. Once the dispatcher enqueues a request, 
 ** it signals for one driver to wake up and service the request. The second CV 
 ** is used (and waited on) by the dispatcher's dispatch method, when 
 ** the queue is at capacity in order to detect that there is more space. 
 ** Most of the critical sections are around manipulations of the request 
 ** queue, so when the dispatcher pushes a request onto the queue or the 
 ** driver pulls a request off of the queue.
 **/

int init_world(world_t* state) {
    // Your code here!

    //Initialize the Mutex
    if (pthread_mutex_init(&(state->mutex), NULL) != 0) {
        perror("Mutex Initialization Error\n");
        exit(1);
    }

    //Initialize the CV
    if (pthread_cond_init(&(state->trip), NULL) != 0) {
        perror("CV Initialization Failed\n");
        exit(1);
    }

    //Initialize the CV
    if (pthread_cond_init(&(state->done), NULL) != 0) {
        perror("CV Initialization Failed\n");
        exit(1);
    }

    //Allocation Space for the Queue
    state->request_queue = (queue_t*) malloc(sizeof(queue_t));
    if (state->request_queue == NULL) {
        perror("Malloc of World State Failed\n");
        exit(1);
    }

    //Initialize the Queue
    queue_init(state->request_queue);
    state->dispatchDone = 0;

    return 0;
}

void* dispatcher_thread(void* arg) {
    world_t* state = (world_t*)arg;
    char *line = NULL;
    size_t nbytes = 0;
    request_t* req = NULL;
    int scanned;

    // DO NOT change the following loop
    while(getline(&line, &nbytes, stdin) != -1) {
        req = (request_t*)malloc(sizeof(request_t));
        // Parse stdin inputs
        scanned = sscanf(line, "%lu %lu %f %f %f %f",
            &req->customer_id, &req->timestamp,
            &req->origin.longitude, &req->origin.latitude,
            &req->destination.longitude, &req->destination.latitude);
        assert(scanned == 6);
        dispatch(state, (void*)req);
        free(line);
        line = NULL;
        nbytes = 0;
    }

    /* Set shared state to indicate dispatcher is done */
    pthread_mutex_lock(&(state->mutex));    
    state->dispatchDone = 1;
    pthread_cond_broadcast(&(state->trip));
    pthread_mutex_unlock(&(state->mutex));

    return NULL;
}

// Implement the actual dispatch() and driver_thread() methods
void dispatch(world_t* state, void* req) {
    
    // While queue is at capacity, wait for signal
    pthread_mutex_lock(&(state->mutex));    
    while (size(state->request_queue) == MAX_QUEUE_SIZE) {
        if (state->dispatchDone == 1) {
            pthread_mutex_unlock(&(state->mutex));
            break;
        }
        pthread_cond_wait(&(state->done), &(state->mutex));
    }
    pthread_mutex_unlock(&(state->mutex));

    // Push request onto queue & send signal to wake up one driver
    pthread_mutex_lock(&(state->mutex));
    if (size(state->request_queue) < MAX_QUEUE_SIZE) {
        push_back(state->request_queue, req);
        pthread_cond_signal(&(state->trip));
    }
    pthread_mutex_unlock(&(state->mutex));

}

void* driver_thread(void* arg) {
    world_t* state = (world_t*)arg;
    struct timeval tv;
    struct timespec ts;

    gettimeofday(&tv, NULL);
    ts.tv_nsec = 0;
    ts.tv_sec = tv.tv_sec + 1;

    // While queue is empty, wait for signal
    pthread_mutex_lock(&(state->mutex));
    while (empty(state->request_queue) == 1) {
        if (state->dispatchDone == 1) {
            pthread_mutex_unlock(&(state->mutex));
            return NULL;    
        }
        if (pthread_cond_timedwait(&(state->trip), &(state->mutex), &ts) != 0)
            perror("Driver: Timed wait error\n");
    }
    pthread_mutex_unlock(&(state->mutex));

    
    while (size(state->request_queue) != 0) {
        // Once request detected, pop off queue & increment counter
        pthread_mutex_lock(&(state->mutex));
        request_t* req = pop_front(state->request_queue);
        pthread_cond_broadcast(&(state->done));
        pthread_mutex_unlock(&(state->mutex));

        // Perform the trip
        drive(req);
        free(req);
    }

    return NULL;
}