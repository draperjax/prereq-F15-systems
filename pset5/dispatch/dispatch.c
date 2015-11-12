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
    (void)state;
    return 0;
}

void* dispatcher_thread(void* arg) {
    world_t* state = (world_t*)arg;
    char *line = NULL;
    size_t nbytes = 0;
    request_t* req;
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
    return NULL;
}

// Implement the actual dispatch() and driver_thread() methods
void dispatch(world_t* state, void* req) {
    // Your code here!
    (void)state; (void)req;
}

void* driver_thread(void* arg) {
    world_t* state = (world_t*)arg;
    // Your code here!
    (void)state;
    return NULL;
}
