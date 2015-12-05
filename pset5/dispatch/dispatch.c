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

// Simulation of a Uber dispatch mechanism
// This is the only file where you need to write your own code!
// Your solution should not rely on modification to any other files!
// See dispatch.h for descriptions of the following functions

int init_world(world_t* state) {
    // Your code here!

    //Initialize the Mutex
    if (pthread_mutex_init(&(state->mutex), NULL) != 0) {
        perror("Mutex Initialization Error\n");
        exit(1);
    }

    //Initialize the CV
    if (pthread_cond_init(&(state->tripEnqueued), NULL) != 0) {
        perror("Trip CV Initialization Failed\n");
        exit(1);
    }

    //Initialize the CV
    if (pthread_cond_init(&(state->tripDequeued), NULL) != 0) {
        perror("Trip CV Initialization Failed\n");
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
    pthread_cond_broadcast(&(state->tripEnqueued));
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

    /* While trips completed is not equal to trips requested, 
    wait for driver's 'done' signal */
    pthread_mutex_lock(&(state->mutex));
    
    // Set shared state to done to indicate nothing left to be done
    state->dispatchDone = 1;
    pthread_cond_broadcast(&(state->tripEnqueued));
    pthread_mutex_unlock(&(state->mutex));

    return NULL;
}

// Implement the actual dispatch() and driver_thread() methods
void dispatch(world_t* state, void* req) {
    
    // Lock the Mutex
    pthread_mutex_lock(&(state->mutex));    

    // While queue is at capacity, wait for driver's 'done' signal
    while (size(state->request_queue) == MAX_QUEUE_SIZE)
        pthread_cond_wait(&(state->tripDequeued), &(state->mutex));

    // Push request onto queue & broadcast 'trip' signal to wake drivers
    if (size(state->request_queue) < MAX_QUEUE_SIZE) {
        push_back(state->request_queue, req);
        pthread_cond_signal(&(state->tripEnqueued));
    }

    pthread_mutex_unlock(&(state->mutex));

}

void* driver_thread(void* arg) {
    world_t* state = (world_t*)arg;
    
    /* With mutex locked, while queue is empty, wait for 
        dispatcher's 'trip' signal */
    pthread_mutex_lock(&(state->mutex));
    while (state->dispatchDone != 1) {

        if (empty(state->request_queue) == 1)
            pthread_cond_wait(&(state->tripEnqueued), &(state->mutex));

        //check if somethings on queue or its done and we should exit
        if (size(state->request_queue) > 0) {
            /* Once request detected, pop off queue & increment counter */
            request_t* req = pop_front(state->request_queue);
            pthread_cond_broadcast(&(state->tripDequeued));

            // Perform the trip
            drive(req);
        }
    }
    pthread_mutex_unlock(&(state->mutex));
    return NULL;
}