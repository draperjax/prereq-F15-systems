 #define M61_DISABLE 1
 #define SZ_MAX 5000
 #include "m61.h"
 #include <stdlib.h>
 #include <string.h>
 #include <stdio.h>
 #include <stdint.h>
 #include <inttypes.h>
 #include <assert.h>
      
 struct m61_statistics user_stats = {
     0, 0, 0, 0, 0, 0, (char*) UINTPTR_MAX, NULL
 };      

 struct statsNode* root = NULL;

 void* m61_malloc(size_t sz, const char* file, int line) {
     (void) file, (void) line;   // avoid uninitialized variable warnings
     // Your code here.
     struct statsNode* metadata_ptr = NULL;
     void* ptr = NULL;
     struct footNode* boundary_ptr = NULL;
    
     if (sz < SZ_MAX) {
         /* Setup metadata pointer to capture malloc's output */
         metadata_ptr = malloc(sizeof(struct statsNode) + sz + sizeof(struct footNode));
         
         /* Setup the pointer to payload */
         ptr = (char*) metadata_ptr + sizeof(struct statsNode);
         
         /* Setup the pointer to the footer */
         boundary_ptr = (struct footNode*) ((char*) ptr + sz);
     }       
          
     /* Check if malloc was successful and increment user_stats */
     if (ptr == NULL) {
         /* Malloc failed to allocate memory */
         user_stats.nfail += 1;
         user_stats.fail_size += (int) sz;
         return NULL;
     } else {
        /* Increment the metadata counts */
        user_stats.nactive += 1;
        user_stats.ntotal += 1;
        
        /* Set the allocation live indicator to 1 */ 
        metadata_ptr->active = 1;
        
        /* Increment the metadata allocation sizes */
        user_stats.active_size += (int) sz; 
        user_stats.total_size += (int) sz; 
        
        /* Define node with current alloc */
        metadata_ptr->ptr = ptr;
        metadata_ptr->sz = (int) sz; 
        
        /* Set Heap Max */
        if (user_stats.heap_max < (char*) ptr + sz + sizeof(struct footNode)) 
            user_stats.heap_max = (char*) ptr + sz + sizeof(struct footNode);
        
        /* Set Heap Min */
        if (user_stats.heap_min > (char*) metadata_ptr)
            user_stats.heap_min = (char*) metadata_ptr;
        
        metadata_ptr->head = 88;
        boundary_ptr->boundary1 = 23;
        
        /* Adding a node to the linked list of metadata pointers */
        struct statsNode* head = malloc(sizeof(struct statsNode));

        if (root == NULL) {
            root = metadata_ptr;
        } else {
            while (root->prev != NULL) {
                metadata_ptr->prev = root;
            }
            
            while (root->next != NULL) {
                root = metadata_ptr->next;
            }
            
            root = metadata_ptr;

//            while (root->next != NULL) {
//                root = metadata_ptr->next;
//            }

//            root->next = metadata_ptr;
//            root->next = (char*) metadata_ptr;
//            metadata_ptr->prev = (char*) root;
//            root = metadata_ptr;
//        } else {
//            *root = metadata_ptr;
        }
 
     return ptr; 
     }    
 }       
     
 void m61_free(void *ptr, const char *file, int line) {
     (void) file, (void) line;   // avoid uninitialized variable warnings
     // Your code here.

        if ((char*) ptr >= (user_stats.heap_max + sizeof(struct footNode)) || (char*) ptr < (user_stats.heap_min + sizeof(struct statsNode))) {
             printf("MEMORY BUG: invalid free of pointer %p, not in heap\n", ptr);
             exit(0);
        } else {
             struct statsNode* metadata_ptr = ((struct statsNode*) ptr - 1);
             size_t sz = metadata_ptr->sz;
             struct footNode* boundary_ptr = (struct footNode*) ((char*) ptr + sz);
        
             if (metadata_ptr->ptr != ptr) {
                 printf("MEMORY BUG: %s:%i: invalid free of pointer %p, not allocated\n", file,line,ptr);
                 int distance = (char*) metadata_ptr->ptr - (char*) ptr;
                 printf("  %s:%i: %p is %i bytes inside a %i byte region allocated here\n", file,(line -1), ptr, distance, sz);
                 exit(0);
             } else if (metadata_ptr->active == 0) {
                 printf("MEMORY BUG: invalid free of pointer %p", ptr);
                 exit(0);
             } else { 
                 if (boundary_ptr->boundary1 != 23) {
                     printf("MEMORY BUG: detected wild write during free of pointer %p, %zu, %llu", ptr, sz, user_stats.active_size); 
                     exit(0);
                 } else {
                     size_t temp_sz = sz;
                     user_stats.active_size -= (int) temp_sz;

                     metadata_ptr->active = 0;

                     /* Decrement active allocations */
                     user_stats.nactive -= 1;

                   /* Free the node from the linked list of metadata pointers */ 
                     if (metadata_ptr == root && root->prev == NULL) {
                         root = NULL;
                     } else if (metadata_ptr == root && root->prev != NULL) {
                         root->prev = NULL;
                         root->next = NULL;
                         root = root->prev;
                     } else if (metadata_ptr == root) {
                         if (metadata_ptr->next != NULL) {
                             (metadata_ptr->next)->prev = metadata_ptr->prev;
                         }

                         if (metadata_ptr->prev != NULL) {
                            (metadata_ptr->prev)->next = metadata_ptr->prev;
                         }

                         root->prev = root;
                         (root->next)->prev = NULL;
                         root->next = NULL;
                     }
                     
                     free(ptr);
                 }
            }
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
         struct statsNode* metadata = ((struct statsNode*) ptr - 1);
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
     // Your code here
     while (root != NULL) {
        root->prev = root;
     }
     //printf("LEAK CHECK: test%s.c:18: allocated object %ptr with size %zu");
 }
