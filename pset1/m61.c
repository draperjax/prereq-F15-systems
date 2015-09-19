 #define M61_DISABLE 1
 #define SZ_MAX 5000
 #include "m61.h"
 #include <stdlib.h>
 #include <string.h>
 #include <stdio.h>
 #include <inttypes.h>
 #include <assert.h>
     
 //struct statsNode *head = NULL;
         
 struct m61_statistics user_stats = {
     0, 0, 0, 0, 0, 0, NULL, NULL
 };      
     
 void* m61_malloc(size_t sz, const char* file, int line) {
     (void) file, (void) line;   // avoid uninitialized variable warnings
     // Your code here.
     struct statsNode* metadata_ptr = NULL;
     void* ptr = NULL;
         
     if (sz < SZ_MAX) {
         /* Setup metadata pointer to capture malloc's output */
         metadata_ptr = malloc(sz + sizeof(struct statsNode));
         
         /* Setup payload ptr */
         ptr = metadata_ptr + sizeof(struct statsNode);
     }       
          
     /* Check if malloc was successful and increment user_stats */
     if (ptr == NULL) {
         /* Malloc failed to allocate memory */
         user_stats.nfail += 1;
         user_stats.fail_size += (unsigned long long) sz;
         return NULL;
     } else {
         /* Increment the metadata counts */
         if (sz>0)
         {
             user_stats.nactive += 1;
             user_stats.ntotal += 1;  
         }
         
         /* Increment the metadata allocation sizes */
         user_stats.active_size += (unsigned long long) sz;
         user_stats.total_size += (unsigned long long) sz;
             
         /* Define node with current alloc */
         metadata_ptr->ptr = ptr;
         metadata_ptr->sz = (unsigned long long) sz;
         
         /* Set Heap Max */
         if (user_stats.heap_max < (char*) ptr + 200) 
             user_stats.heap_max = (char*) ptr + 200;
     
         /* Set Heap Min */
         if (user_stats.heap_min > (char*) ptr - 10)
             user_stats.heap_min = (char*) ptr - 10;
         else if (user_stats.heap_min == NULL)
             user_stats.heap_min = ((char*) ptr - 10);
     
         return ptr; 
     }    
 }       
     
 void m61_free(void *ptr, const char *file, int line) {
     (void) file, (void) line;   // avoid uninitialized variable warnings
     // Your code here. 

     if ((char*) ptr > user_stats.heap_min) {
         struct statsNode* metadata_ptr = (struct statsNode*) ptr - sizeof(struct statsNode);
         if ((char*) ptr > user_stats.heap_max) {
             printf("MEMORY BUG: invalid free of pointer %p, not in heap", ptr);
             exit(0);
         } else if (metadata_ptr->ptr != ptr) {
             printf("MEMORY BUG: %s:%i: invalid free of pointer %p, not allocated", file,line,ptr);
             exit(0);
         } else if (metadata_ptr->sz == (unsigned long long) NULL) { 
             printf("MEMORY BUG: invalid free of pointer %p", ptr);
             exit(0);
         } else {
             unsigned long long temp_sz = metadata_ptr->sz;
             user_stats.active_size -= temp_sz;
             
             /* Decrement active allocations */
             user_stats.nactive -= 1;
             
             metadata_ptr->sz = (unsigned long long) NULL;

             free(ptr);
         }
     } else {
         struct statsNode* metadata = ((struct statsNode*) ptr)-sizeof(struct statsNode);
         unsigned long long temp_sz = sizeof(*metadata);
         user_stats.active_size -= temp_sz;
         
         /* Decrement active allocations */
         user_stats.nactive -= 1;
         free(ptr);
     }
     
 }       
         
 void* m61_realloc(void* ptr, size_t sz, const char* file, int line) {
     void* new_ptr = NULL;
     
     if (sz > 0)
         new_ptr = m61_malloc(sz, file, line);
     if (ptr && new_ptr) {
         // Copy the data from `ptr` into `new_ptr`.
         // To do that, we must figure out the size of allocation `ptr`.
         // Your code here (to fix test012)
         struct statsNode* metadata = ((struct statsNode*) ptr)-sizeof(struct statsNode);
         size_t ptr_sz = metadata->sz;
        
         if (sz > ptr_sz) {
             memcpy(new_ptr,ptr,ptr_sz);
         } else {
             memcpy(new_ptr,ptr,sz);
         }
     }
     if (ptr != NULL)
         m61_free(ptr, file, line);
     return new_ptr;
 }       
         
 void* m61_calloc(size_t nmemb, size_t sz, const char* file, int line) {
     // Your code here (to fix test014).
     void* ptr = NULL;
     if (sz < SZ_MAX)
         ptr = m61_malloc(nmemb * sz, file, line);
     else
         user_stats.nfail += 1;
 
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

