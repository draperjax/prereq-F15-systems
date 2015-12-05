#include <pthread.h>
#include <stdio.h>
#include <errno.h>
#include <stdbool.h>
#include <assert.h>
#include <sys/time.h>
#include <time.h>
#include <math.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

// timestamp()
//    Return the current time as a double.

/* -- EXPLANATION -- 
Better Init:
Synchronization strategy: Leverage Randomization to decrease the likelihood 
of a passenger waiting on a specific uber car (i.e.) choose any of the fleet 
that may be available.

Correctness criteria/Why it works: This is a correct better initialization 
for uber-pi because it introduces randomization in the choice of the uber car. 
This strategy increases the likelihood that the passenger is not waiting for 
a specific uber car, and thereby decreases the time passengers are waiting 
to acquire the lock and be driven.

Trylock:
Synchronization Strategy: Leverage Trylocks & Randomization to decrease the 
likelihood of a passenger waiting on a specific uber car and to avoid the 
passenger being blocked from checking other cars while waiting.
Correctness criteria/Why it works: This is a correct trylock initialization 
for uber-pi because it tests all calls in the uber fleet without waiting while 
blocked, first, and then once it discovers all are busy, it waits on one 
(to avoid endless trylock attempts).
*/

static inline double timestamp(void) {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec + tv.tv_usec / 1000000.0;
}

#define NUM_THREADS 8
#define NUM_PASSENGERS NUM_THREADS
#define NUM_UBERS 4
#define CALCS_PER_UBER 1500000
#define RIDES_PER_PASSENGER 3
#define PI 3.14159265358979323846264338327

pthread_mutex_t uber_locks[NUM_UBERS];
double uber_times[NUM_UBERS];
volatile double inside[NUM_UBERS];
volatile double outside[NUM_UBERS];

int in_use;

static inline double rand_unif() {
	return (double)rand() / (double)RAND_MAX;
}

void drive(int thread_id, int uber_id) {
	(void) thread_id;

    double start_time = timestamp();
	double sample_x;
	double sample_y;
	double res;
	for (int k = 0; k < CALCS_PER_UBER; ++k) {
		sample_x = rand_unif();
		sample_y = rand_unif();

		res = pow(sample_x, 2) + pow(sample_y, 2);
		if (res < 1.0) {
			inside[uber_id]++;
		}
		else {
			outside[uber_id]++;
		}
	}

    uber_times[uber_id] += (timestamp() - start_time);

}

// Question to be answered in your README: Why is this function so slow?
void* passenger(void* params) {
	int me = (int)params;
	for (int k = 0; k < RIDES_PER_PASSENGER; ++k) {
		for (int i = 0; i < NUM_UBERS; ++i) {
			pthread_mutex_lock(&uber_locks[i]);
			drive(me, i);
			pthread_mutex_unlock(&uber_locks[i]);
			break;
		}
	}
	return NULL;
}


void* passenger_better_init(void* params) {
	int me = (int)params;
	//randomize choice of uber car to avoid waiting on a specific car
	for (int k = 0; k < RIDES_PER_PASSENGER; ++k) {
		int uber = (me % NUM_UBERS);
		pthread_mutex_lock(&uber_locks[uber]);
		drive(me, uber);
		pthread_mutex_unlock(&uber_locks[uber]);
	}
	return NULL;
}

void* passenger_trylock(void* params) {
	int me = (int)params;

	for (int k = 0; k < RIDES_PER_PASSENGER; ++k) {
		int uber = 0;
		//attempt a trylock and if it fails, check others until tested all
		while (true) {
			if (pthread_mutex_trylock(&uber_locks[uber]) == 0) {
				drive(me, uber);
				pthread_mutex_unlock(&uber_locks[uber]);
				break;
			}

			uber++;
			uber = uber % NUM_UBERS;
		}
	}
	return NULL;
}

static void print_usage() {
	printf("Usage: ./uber-pi [PASSENGER_TYPE]\n");
	exit(1);
}

int main (int argc, char** argv) {
	srand((unsigned)time(NULL));
	pthread_t threads[NUM_THREADS];

	if (argc < 2) {
		print_usage();
	}

	for (int j = 0; j < NUM_UBERS; ++j) {
		pthread_mutex_init(&uber_locks[j], NULL);
	}

    double timevar = timestamp();

	for (long long i = 0; i < NUM_PASSENGERS; ++i) {
		if (strcmp(argv[1], "2") == 0) {
			if (pthread_create(&threads[i], NULL, passenger_trylock, (void *)i)) {
				printf("pthread_create failed\n");
				exit(1);
			}
		}
		else if (strcmp(argv[1], "1") == 0) {
			if (pthread_create(&threads[i], NULL, passenger_better_init, (void *)i)) {
				printf("pthread_create failed\n");
				exit(1);
			}
		}
		else if (strcmp(argv[1], "0") == 0) {
			if (pthread_create(&threads[i], NULL, passenger, (void *)i)) {
				printf("pthread_create failed\n");
				exit(1);
			}
		}
		else {
			print_usage();
		}
	}

	for (int i = 0; i < NUM_PASSENGERS; ++i) {
		pthread_join(threads[i], NULL);
	}

    timevar = (timestamp() - timevar);

	double inside_sum = 0;
	double outside_sum = 0;
    double total_uber_time = 0.0;
	for (int u = 0; u < NUM_UBERS; ++u) {
		inside_sum += inside[u];
		outside_sum += outside[u];
        total_uber_time += uber_times[u];
	}

	double mc_pi = 4.0 * inside_sum/(inside_sum + outside_sum);

    printf("Average fraction of time Uber drivers were driving: %5.3f\n",
            (total_uber_time / NUM_UBERS) / timevar);
	printf("Value of pi computed was: %f\n", mc_pi);
	if (!(fabs(mc_pi - PI) < 0.02)) {
		printf("Your computation of pi was not very accurate, something is probably wrong!\n");
		exit(1);
	}
	return 0;
}
