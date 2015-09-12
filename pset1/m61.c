#define M61_DISABLE 1
#include "m61.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <inttypes.h>
#include <assert.h>

struct m61_user_alloc_stats temp_stats = { 
    0
};

struct m61_statistics user_stats = {
    0, 0, 0, 0, 0, 0, NULL, NULL
};

void* m61_malloc(size_t sz, const char* file, int line) {
    (void) file, (void) line;   // avoid uninitialized variable warnings
    // Setup pointer to capture malloc's output
    void* mem_active_ptr = malloc(sz + 1); 
    // Check if malloc ran successfully and if so, increment the user_stats struct
    if (mem_active_ptr == NULL) {
        //malloc failed to allocate memory
        user_stats.nfail += 1;
        user_stats.fail_size += ((unsigned long long) sz);
        return NULL;
    } else {
        user_stats.nactive += 1;
        user_stats.ntotal += 1;
        
        temp_stats.sz = ((unsigned long long) sz);
        
        user_stats.active_size += ((unsigned long long) sz);
        user_stats.total_size += ((unsigned long long) sz);
        
        return mem_active_ptr;
    }
}

void m61_free(void *mem_active_ptr, const char *file, int line) {
    (void) file, (void) line;   // avoid uninitialized variable warnings
    // Check if free ran successfully, and if so decrement user_stats struct
    
    unsigned long long sz = temp_stats.sz;
    
    user_stats.nactive -= 1;
    user_stats.active_size -= sz;
    
    free(mem_active_ptr);
}

void* m61_realloc(void* ptr, size_t sz, const char* file, int line) {
    void* new_ptr = NULL;
    if (sz)
        new_ptr = m61_malloc(sz, file, line);
    if (ptr && new_ptr) {
        // Copy the data from `ptr` into `new_ptr`.
        // To do that, we must figure out the size of allocation `ptr`.
        // Your code here (to fix test012).
    }
    m61_free(ptr, file, line);
    return new_ptr;
}

void* m61_calloc(size_t nmemb, size_t sz, const char* file, int line) {
    // Your code here (to fix test014).
    void* ptr = m61_malloc(nmemb * sz, file, line);
    if (ptr)
        memset(ptr, 0, nmemb * sz);
    return ptr;
}


void m61_getstatistics(struct m61_statistics* stats) {
    // Stub: set all statistics to enormous numbers
    memset(stats, 255, sizeof(struct m61_statistics));
    // Your code here.
    *stats = user_stats;
}

void m61_printstatistics(void) {
    struct m61_statistics stats;
    m61_getstatistics(&stats);

    printf("malloc count: active %10llu   total %10llu   fail %10llu\n",
           stats.nactive, stats.ntotal, stats.nfail);
    printf("malloc size:  active %10llu   total %10llu   fail %10llu\n",
           stats.active_size, stats.total_size, stats.fail_size);
}

void m61_printleakreport(void) {
    // Your code here.
}
