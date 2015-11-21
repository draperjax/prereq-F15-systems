// Simulation of a Uber dispatch mechanism
// This is the only file where you need to write your own code!
// Your solution should not rely on modification to any other files!
// See dispatch.h for descriptions of the following functions
#include "dispatch.h"
/**
 ** Describe your synchronization strategy here:
 **
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

    //Allocation Space for the Queue
    state->request_queue = (queue_t*) malloc(sizeof(queue_t));
    if (state->request_queue == NULL) {
        perror("Malloc of World State Failed\n");
        exit(1);
    }

    //Initialize the Queue
    queue_init(state->request_queue);

    //Intialize the Counters
    state->finished = 0;
    state->numTripsRequested = 0;
    state->numTripsComplete = 0;
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

    // Lock the Mutex
    pthread_mutex_lock(&(state->mutex));
    
    /* While trips completed is less than trips requested, 
    wait for driver's 'done' signal */
    while(state->numTripsComplete < state->numTripsRequested) {
        pthread_cond_wait(&(state->done), &(state->mutex));
    }

    state->finished = 1;

    // Unlock the Mutex
    pthread_mutex_unlock(&(state->mutex));

    return NULL;
}

// Implement the actual dispatch() and driver_thread() methods
void dispatch(world_t* state, void* req) {
    
    // Lock the Mutex
    pthread_mutex_lock(&(state->mutex));    
    // Increment the counter of trips requested
    state->numTripsRequested++;

    // While queue is at capacity, wait for driver's 'done' signal
    while (size(state->request_queue) == MAX_QUEUE_SIZE)
        pthread_cond_wait(&(state->done), &(state->mutex));

    // Push request onto queue & broadcast 'trip' signal to wake drivers
    if (size(state->request_queue) < MAX_QUEUE_SIZE) {
        push_back(state->request_queue, req);
        pthread_cond_signal(&(state->trip));
    }

    pthread_mutex_unlock(&(state->mutex));

}

void* driver_thread(void* arg) {
    world_t* state = (world_t*)arg;
    
    // Lock the Mutex
    pthread_mutex_lock(&(state->mutex));
    
    /* While queue is empty & trips completed is not equal to trips requested,
    wait for dispatcher's 'trip' signal */
    while (empty(state->request_queue) == 1)
        pthread_cond_wait(&(state->trip), &(state->mutex));
    pthread_mutex_unlock(&(state->mutex));

    while (empty(state->request_queue) == 0) {
        // Pop request off the front of the queue & increment counter
        pthread_mutex_lock(&(state->mutex));
        request_t* req = pop_front(state->request_queue);
        state->numTripsComplete++;
        pthread_cond_broadcast(&(state->done));
        pthread_mutex_unlock(&(state->mutex));

        // Perform the trip
        drive(req);

        usleep(2);
    }

    return NULL;
}