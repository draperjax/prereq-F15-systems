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

/**
 * Describe your synchronization strategy here:
 *   
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
            customer_get_meal(customer, &meal);


            // you might want some synchronization here for parts one and two...
            // you might want some checks here for part two...

            struct restaurant *restaurant = NULL;
            meal_get_restaurant(meal, &restaurant);

            struct location *srcLocation = NULL;
            customer_get_location(customer, &srcLocation);

            struct location *destLocation = NULL;
            restaurant_get_location(restaurant, &destLocation);

            // you might want some synchronization here for part one...

            if (customer_picked_up(customer) == 0 &&
                meal_picked_up(meal) == 0)
            {
                driver_drive_to_location(driver, srcLocation);
                driver_pick_customer_up(driver, customer);
                driver_drive_to_location(driver, destLocation);
                driver_drop_off_customer(driver, customer, restaurant);
            }

            // you might want some synchronization here for parts one and two...
        }
        else
        {
            struct meal *meal = (struct meal *)request;

            // you might want some synchronization here for part two...

            struct customer *customer = NULL;
            meal_get_customer(meal, &customer);


            // you might want some synchronization here for part two...
            // you might want some checks here for part two...
            
            struct restaurant *restaurant = NULL;
            meal_get_restaurant(meal, &restaurant);

            struct location *srcLocation = NULL;
            restaurant_get_location(restaurant, &srcLocation);

            struct location *destLocation = NULL;
            customer_get_location(customer, &destLocation);

            // you might want some synchronization here for part one...

            if (meal_picked_up(meal) == 0 &&
                customer_picked_up(customer) == 0)
            {
                driver_drive_to_location(driver, srcLocation);
                driver_pick_meal_up(driver, meal);
                driver_drive_to_location(driver, destLocation);
                driver_drop_off_meal(driver, meal, customer);
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

    meal->customer = NULL;
    meal->restaurant = NULL;

}
void cleanup_meal(struct meal *meal)
{
    private_tracking_destroy(meal->stats);

}

// init/cleanup customer
void init_customer(struct customer *customer)
{
    customer->stats = private_tracking_create();

    customer->meal = NULL;
    customer->location = NULL;

}
void cleanup_customer(struct customer *customer)
{
    private_tracking_destroy(customer->stats);

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
