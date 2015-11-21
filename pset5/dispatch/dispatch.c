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

    if (pthread_mutex_init(&(state->mutex), NULL) != 0) {
        perror("Mutex Initialization Error\n");
        exit(1);
    }
    if (pthread_cond_init(&(state->trip), NULL) != 0) {
        perror("CV Initialization Failed\n");
        exit(1);
    }

    state->request_queue = (queue_t*) malloc(sizeof(queue_t));
    if (state->request_queue == NULL) {
        perror("Malloc of World State Failed\n");
        exit(1);
    }

    queue_init(state->request_queue);
    state->drivers_done = 0;

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

    // Your code here!
    // pthread_mutex_lock(&(state->mutex));
    // while(state->drivers_done < 3) {
    //     pthread_cond_wait(&(state->done), &(state->mutex));
    // }
    // pthread_mutex_unlock(&(state->mutex));
    return NULL;
}

// Implement the actual dispatch() and driver_thread() methods
void dispatch(world_t* state, void* req) {
    // Your code here!

    pthread_mutex_lock(&(state->mutex));
    if (size(state->request_queue) < MAX_QUEUE_SIZE) {
        push_back(state->request_queue, req);
        pthread_cond_broadcast(&(state->trip));
    } else {
        pthread_cond_wait(&(state->done), &(state->mutex));
        push_back(state->request_queue, req);
        pthread_cond_broadcast(&(state->trip));
    }
    pthread_mutex_unlock(&(state->mutex));
}

void* driver_thread(void* arg) {
    world_t* state = (world_t*)arg;
    // Your code here!
    pthread_mutex_lock(&(state->mutex));
    while (empty(state->request_queue) == 1) {
        pthread_cond_wait(&(state->trip), &(state->mutex));
    }
    request_t* req = pop_front(state->request_queue);
    state->drivers_done++;
    pthread_mutex_unlock(&(state->mutex));

    drive(req);    

    pthread_mutex_lock(&(state->mutex));
    pthread_cond_broadcast(&(state->done));
    pthread_mutex_unlock(&(state->mutex));
    
    return NULL;
}