#pragma once

#include <stdlib.h>
#include "ubler_helpers.h"

/**
 * The definitions of meal and customer. You will probably have to change
 * these structs.
 */
struct meal
{
    struct private_tracking *stats;

    struct customer *customer;
    struct restaurant *restaurant;

    // add other things as you go along
    #if SOL >= 3
    pthread_mutex_t meal_lock;
    #endif
};

struct customer
{
    struct private_tracking *stats;

    struct meal *meal;
    struct location *location;

    // add other things as you go along
    #if SOL >= 3
    pthread_mutex_t customer_lock;
    #endif
};

// init/cleanup meal
void init_meal(struct meal *meal);
void cleanup_meal(struct meal *meal);

// init/cleanup customer
void init_customer(struct customer *customer);
void cleanup_customer(struct customer *customer);

void *driver_thread(void *driver_arg);
