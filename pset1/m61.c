#define M61_DISABLE 1
#include "m61.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <inttypes.h>
#include <assert.h>

struct statsNode *head = NULL;

struct m61_statistics user_stats = {
    0, 0, 0, 0, 0, 0, NULL, NULL 
};

void* m61_malloc(size_t sz, const char* file, int line) {
    (void) file, (void) line;   // avoid uninitialized variable warnings
    // Your code here.
    /* Setup pointer to capture malloc's output */
    void* ptr = malloc(sz); 
    /* Check if malloc was successful and increment user_stats */
    if (ptr == NULL) {
        /* Malloc failed to allocate memory */
        user_stats.nfail += 1;
        user_stats.fail_size += ((unsigned long long) sz);
        return NULL;
    } else {
        /* Increment the metadata counts */
        user_stats.nactive += 1;
        user_stats.ntotal += 1;  
        
        /* Increment the metadata allocation sizes */
        user_stats.active_size += ((unsigned long long) sz);
        user_stats.total_size += ((unsigned long long) sz);

        /* Setup LL Node to store ptr/alloc info */
        struct statsNode* current = NULL;

        /* Define node with current alloc */
        current = malloc(sizeof(struct statsNode));
        current->ptr = ptr;
        current->sz = (unsigned long long) sz;
        current->next = head;
        head = current;
        
        /* Set Heap Max */
        if (user_stats.heap_max < (char*) ptr + 100)
            user_stats.heap_max = (char*) ptr + 100;

        /* Set Heap Min */
        if (user_stats.heap_min == NULL && 
            user_stats.heap_min > (char*) ptr - 100)
            user_stats.heap_min = (char*) ptr - 100;
        else if (user_stats.heap_min == NULL)
            user_stats.heap_min = ((char*) ptr - 100);

        return ptr;
    }    
}

void m61_free(void *ptr, const char *file, int line) {
    (void) file, (void) line;   // avoid uninitialized variable warnings
    // Your code here.
    /* Decrement active allocations */
    user_stats.nactive -= 1;

    struct statsNode* current = head;
    
    /* Iterate through Linked List to check ptrs */
    /* Decrement active_size if mem_active_ptr matches */
    /* Format guided by Nick Parlante's 'Linked List Basics' */
    while (current != NULL)
    {
        if (current->ptr == ptr)
        {
            unsigned long long temp_sz = current->sz;
            user_stats.active_size -= temp_sz;
        }
        current = current->next;
    }
    
    free(ptr);
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
