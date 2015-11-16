README for CS 61 Problem Set 5
------------------------------
OTHER COLLABORATORS AND CITATIONS (if any):



NOTES FOR THE GRADER (if any):

WP-1. Binary Semaphore
WP-2. Counting Semaphore
WP-3. Binary Semaphore
WP-4. Lock without a condition variable
WP-5. Lock and a condition variable
WP-6. At any point in time, only one car is ever utilized, so a passenger is always waiting for one specific car. This causes 25% utilization (i.e. only 1 of the 4 cars are ever in operation). All other passengers must wait after calling lock on a locked mutex, until the mutex is unlocked. 
WP-7. !!!Conditions trylock outperforms simple randomization.!!!