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
    pthread_mutex_t mutex;
    struct customer *customer;
    struct restaurant *restaurant;

    // add other things as you go along
};

struct customer
{
    struct private_tracking *stats;
    pthread_mutex_t mutex;
    struct meal *meal;
    struct location *location;

    // add other things as you go along
};

// init/cleanup meal
void init_meal(struct meal *meal);
void cleanup_meal(struct meal *meal);

// init/cleanup customer
void init_customer(struct customer *customer);
void cleanup_customer(struct customer *customer);

void *driver_thread(void *driver_arg);
