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
		// ptr = (char*) (metadata_ptr + 1);
		ptr = (void*) ((char*) metadata_ptr + sizeof(struct statsNode));
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
		metadata_ptr->file = file;
		metadata_ptr->line = line;
		
		/* Increment the metadata allocation sizes */
		user_stats.active_size += (int) sz; 
		user_stats.total_size += (int) sz; 
		
		/* Define node with current alloc */
		metadata_ptr->ptr = (char*) ptr;
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
		//struct statsNode* head = malloc(sizeof(struct statsNode));

		if (root != NULL) {
			root->next = metadata_ptr;
			metadata_ptr->prev = root;
			root = metadata_ptr;
		} else {
			root = metadata_ptr;
		} 
	}
	return ptr;
 }       
	
 void m61_free(void *ptr, const char *file, int line) {
	(void) file, (void) line;   // avoid uninitialized variable warnings
	// Your code here.

		if ((char*) ptr > (user_stats.heap_max + sizeof(struct footNode)) || (char*) ptr < (user_stats.heap_min + sizeof(struct statsNode))) {
			printf("MEMORY BUG: invalid free of pointer %p, not in heap\n", ptr);
			exit(0);
		} else {
			struct statsNode* metadata_ptr = (struct statsNode*) ((char*) ptr - sizeof(struct statsNode));
			struct footNode* boundary_ptr = (struct footNode*) ((char*) ptr + metadata_ptr->sz);
		
			if (metadata_ptr->ptr != ptr) {
				printf("MEMORY BUG: %s:%i: invalid free of pointer %p, not allocated\n", file,line,ptr);

				size_t distance = ((size_t) ptr - (size_t)(root->ptr));
				printf("  %s:%i: %p is %zu bytes inside a %llu byte region allocated here\n", file,(line -1), ptr,distance, user_stats.active_size);
				exit(0);
			} else if (metadata_ptr->active == 0) {
				printf("MEMORY BUG: invalid free of pointer %p", ptr);
				exit(0);
			} else if (boundary_ptr->boundary1 != 23) {
				printf("MEMORY BUG: detected wild write during free of pointer %p", ptr); 
				exit(0);
			} else {
				size_t temp_sz = metadata_ptr->sz;
				user_stats.active_size -= (int) temp_sz;

				metadata_ptr->active = 0;
				metadata_ptr->head = 25;

				/* Decrement active allocations */
				user_stats.nactive -= 1;

				if (root == metadata_ptr) {
					if (root->next) {
						((struct statsNode*)root->next)->prev = root->prev;
					}

					if (root->prev) {
						((struct statsNode*)root->prev)->next = root->next;
					}

					root = root->prev;
				}

				struct statsNode* iter = root;
				while (iter != NULL){
					// covers the case of first, last and somewhere is the middle 
					if (iter == metadata_ptr){
						if (iter->next) {
							((struct statsNode*)iter->next)->prev = iter->prev;
						}

						if (iter->prev) {
							((struct statsNode*)iter->prev)->next = iter->next; 
						}
						iter = NULL;
					} else {
						iter = iter->prev;
					}
				}
				free(ptr);
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

	struct statsNode* traverse = root;
	while (traverse != NULL) {
		size_t sz = traverse->sz;
		struct statsNode* ptr = traverse->ptr;
		const char* file = traverse->file;
		int line = traverse->line;
		printf("LEAK CHECK: %s:%i: allocated object %p with size %zu\n", file, line, ptr, sz);
		traverse = traverse->prev;
	}
 }
