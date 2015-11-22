/**
 * The main thread for Ubler drivers.
 *
 * Assignment:
 *   Synchronize multiple driver threads as they pick up meals or customers.
 *   You'll need some sort of locking mechanisms as drivers try to access the same
 *     meals or customers, but you'll also need to be careful about the interaction
 *     between meals and customers.  Be careful that your solution doesn't create
 *     deadlock.
 *
 * Extra challenge:
 *   Deal with restaurants who mix their customers orders up.  You'll need to call
 *     fix_mismatch to fix the mixup.  Of course, fix_mismatch
 *     isn't synchronized, so you'll have to synchronize it!  Be careful about which
 *     meals and which customers you've already synchronized when you try to do this.
 *   If you'd like to try this part out, you can pass the --mix-up-meals option
 *     to ubler_test to have 50% of the restaurants mix their meals up.  You can
 *     also run make extra to run tests that automatically test this functionality.
 *   
 */

#include <stdlib.h>
#include <stdbool.h>
#include <pthread.h>
#include "ubler.h"
#include "ubler_helpers.h"
#include <stdio.h>

/**
 * Describe your synchronization strategy here:
 * My strategy leverages two mutexes that lock the meal struct and customer 
 * struct individually. The idea here is that prior to departing, the driver 
 * must make sure that the other part of the request (the part he/she didn't 
 * receive) is not en route, so every check involving that object should be 
 * protected. Only after that should the driver then first lock the part of 
 * the request he doesn't have, then lock the part of the request he is dropping 
 * off, complete the drop-off and then release the lock on the part he doesn't 
 * have and then release the lock on the part he has.
 */

// Ignore this function unless you are doing the extra challenge
int fix_mismatch(struct meal *mealA);


/**
 * The main driver thread.  This is where most of your changes should go.
 * You should also change the meal/customer init/cleanup functions, and the
 *   meal and customer structs
 */
void *driver_thread(void *driver_arg)
{
    struct driver *driver = (struct driver *)driver_arg;

    while (1)
    {
        void *request;
        bool isCustomer;
        int result = receive_request(&request, &isCustomer);
        if (result != 0)
        {
            // no more destinations left!
            break;
        }
        if (isCustomer)
        {
            struct customer *customer = (struct customer *)request;
            
            // you might want some synchronization here for part two...

            struct meal *meal = NULL;
            pthread_mutex_lock(&(customer->mutex));
            customer_get_meal(customer, &meal);
            pthread_mutex_unlock(&(customer->mutex));

            // you might want some synchronization here for parts one and two...
            // you might want some checks here for part two...

            struct restaurant *restaurant = NULL;
            pthread_mutex_lock(&(meal->mutex));            
            meal_get_restaurant(meal, &restaurant);
            pthread_mutex_unlock(&(meal->mutex));            

            struct location *srcLocation = NULL;
            pthread_mutex_lock(&(customer->mutex));
            customer_get_location(customer, &srcLocation);
            pthread_mutex_unlock(&(customer->mutex));

            struct location *destLocation = NULL;
            restaurant_get_location(restaurant, &destLocation);

            // you might want some synchronization here for part one...

            if (customer_picked_up(customer) == 0 &&
                meal_picked_up(meal) == 0)
            {
                pthread_mutex_lock(&(meal->mutex));
                pthread_mutex_lock(&(customer->mutex));
                driver_drive_to_location(driver, srcLocation);
                driver_pick_customer_up(driver, customer);
                driver_drive_to_location(driver, destLocation);
                driver_drop_off_customer(driver, customer, restaurant);
                pthread_mutex_unlock(&(meal->mutex));
                pthread_mutex_unlock(&(customer->mutex));
            }

            // you might want some synchronization here for parts one and two...
        }
        else
        {
            struct meal *meal = (struct meal *)request;

            // you might want some synchronization here for part two...

            struct customer *customer = NULL;
            pthread_mutex_lock(&(meal->mutex));
            meal_get_customer(meal, &customer);
            pthread_mutex_unlock(&(meal->mutex));


            // you might want some synchronization here for part two...
            // you might want some checks here for part two...
            
            struct restaurant *restaurant = NULL;
            pthread_mutex_lock(&(meal->mutex));
            meal_get_restaurant(meal, &restaurant);
            pthread_mutex_unlock(&(meal->mutex));

            struct location *srcLocation = NULL;
            restaurant_get_location(restaurant, &srcLocation);

            struct location *destLocation = NULL;
            pthread_mutex_lock(&(customer->mutex));
            customer_get_location(customer, &destLocation);
            pthread_mutex_unlock(&(customer->mutex));

            // you might want some synchronization here for part one...

            if (meal_picked_up(meal) == 0 &&
                customer_picked_up(customer) == 0)
            {
                pthread_mutex_lock(&(customer->mutex));
                pthread_mutex_lock(&(meal->mutex));
                driver_drive_to_location(driver, srcLocation);
                driver_pick_meal_up(driver, meal);
                driver_drive_to_location(driver, destLocation);
                driver_drop_off_meal(driver, meal, customer);
                pthread_mutex_unlock(&(customer->mutex));
                pthread_mutex_unlock(&(meal->mutex));
            }

            // you might want some synchronization here for parts one and two...
        }
    }

    return NULL;
}

/**
 * Some meal/customer code you may want to change
 */
// init/cleanup meal
void init_meal(struct meal *meal)
{
    meal->stats = private_tracking_create();

    if (pthread_mutex_init(&(meal->mutex), NULL) != 0) {
        perror("Mutex Initialization Error\n");
        exit(1);
    }

    meal->customer = NULL;
    meal->restaurant = NULL;

}
void cleanup_meal(struct meal *meal)
{
    private_tracking_destroy(meal->stats);
    pthread_mutex_destroy(&(meal->mutex));

}

// init/cleanup customer
void init_customer(struct customer *customer)
{
    customer->stats = private_tracking_create();

    if (pthread_mutex_init(&(customer->mutex), NULL) != 0) {
        perror("Mutex Initialization Error\n");
        exit(1);
    }

    customer->meal = NULL;
    customer->location = NULL;

}
void cleanup_customer(struct customer *customer)
{
    private_tracking_destroy(customer->stats);
    pthread_mutex_destroy(&(customer->mutex));

}

// Ignore this function unless you are doing the extra challenge
// All the mismatches are of the following form:
//   customer A wants meal A, and customer A has a pointer to meal A
//   customer B wants meal B, and customer B has a pointer to meal B
//   BUT meal A has a pointer to customer B, and meal B has a pointer to customer A
//   
// you will probably have to change this function for part two, maybe even use a trylock...
// 
// This function returns 0 on success and -1 on failure
int fix_mismatch(struct meal *mealA)
{
    struct customer *customerB = NULL;
    meal_get_customer(mealA, &customerB);
    

    struct meal *mealB= NULL;
    customer_get_meal(customerB, &mealB);

    struct customer *customerA = NULL;
    meal_get_customer(mealB, &customerA);

    meal_set_customer(mealA, customerA);
    meal_set_customer(mealB, customerB);

    return 0;
}
