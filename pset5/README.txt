README for CS 61 Problem Set 5
------------------------------
OTHER COLLABORATORS AND CITATIONS (if any):



NOTES FOR THE GRADER (if any):

WP-1. Lock (without a condition variable). Since only one jumper should be able to 'lock' the jump, and then complete it, there shouldn't be a need to signal to other jumps while the jump is still in play. Thus, a lock on the jump would allow a single player to jump for the ball and then unlock that play himself. 
WP-2. Lock and a condition variable. Since the pancakes come in batches and we want every teen to be able to consume a complete batch as the batch occurs, once a complete batch is made we can signal to allow one teen to proceed and consume the batch.
WP-3. Binary Semaphore, because this is a distributed relay and each runner must not start until the previous runner has completed his/her lap, and the runners are not able to physically hand over the baton, a reasonable synchronization primitive to leverage here would be a binary semaphore since the giving of the resource & the taking of the resource are decoupled. Each runner 'hand' control to allow the next runner to run. A lock with a condition variable could also work, but I'd probably leverage two condition variables to force proper ordering of runners, since signaling cannot control which runner will wake up and take control (i.e. continue the race). Additionally, once the signal was received, the next runner would have to wait for the previous runner to release the lock so they can acquire the lock and proceed, which would be less efficient.
WP-4. Counting Semaphore, because the Cambridge DPW wants the check-ins to be able to allocate and deallocate spots in parallel but they must operate relative to a finite number of tokens. A counting semaphore would allow them to continue operations in parallel, counting the tokens available and then cause all check-ins to wait while there are no more tokens to distribute (until more are added).
WP-5. Lock and a condition variable, because the grad student dilemma is that they lose the signal when they transition from checking to researching (i.e. waiting), implementing a mutex with a CV would allow them to avoid losing the signal as they will wait for the signal, and can go get free food once they detect the signal.
WP-6. At any point in time, only one car is ever utilized, so a passenger is always waiting for one specific car. This causes 25% utilization (i.e. only 1 of the 4 cars are ever in operation). 
WP-7. A trylock can outperform earlier implementations when there are more drivers than passenger requests, so you can keep checking drivers and be very likely to find one driver that is free where you can obtain the lock.