/**
 * The tester class for ubler. Just reads in input from the terminal and runs
 * create_and_run_requests, then checks the output stats.
 */

#include "ubler.h"
#include "ubler_helpers.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

bool check_equal(const char *msg, int got, int expected)
{
    if (expected != got)
    {
        printf("Error: %s mismatch, got %d, expected %d\n", msg, got, expected);
        return true;
    }
    return false;
}

int main(int num_args, char *argv[])
{
    int num_requests = 10;
    int num_drivers = 1;
    int num_restaurants = 5;
    struct ubler_stats stats = {0,0,0,0,0,0,0,0,0,0};
    struct ubler_params params = {10, false};

    if (num_args == 1)
    {
        printf("Usage: ubler_test num_requests num_drivers num_restaurants [--mix-up-meals]\n");
        printf("Using defaults: 10 requests, 1 driver, 5 restaurants, no mix up\n\n");
    }
    else if(num_args >= 4 && num_args <= 5)
    {
        num_requests = atoi(argv[1]);
        num_drivers = atoi(argv[2]);
        num_restaurants = atoi(argv[3]);
        if (num_args == 5 &&
            strncmp("--mix-up-meals", argv[4], strlen("--mix-up-meals")) == 0)
        {
            params.mixUpMeals = true;
        }
    }
    else
    {
        printf("Usage: ubler_test num_requests num_drivers num_restaurants\n");
        return 0;
    }

    int result = create_and_run_requests(num_requests, num_restaurants, num_drivers, &params, &stats);
    if (result > 0)
    {
        printf("Parameters too high\n");
        return 1;
    }

    bool error = false;

    error = check_equal("Requests picked up", stats.total_meals_picked_up + stats.total_customers_picked_up, num_requests) || error;
    check_equal("Requests ignored", stats.total_meals_ignored + stats.total_customers_ignored, 0) || error;
    check_equal("Requests overserviced", stats.total_meals_overserviced + stats.total_customers_overserviced, 0) || error;
    check_equal("Driver pickups, requests", stats.driver_meals_picked_up + stats.driver_customers_picked_up, num_requests) || error;
    check_equal("Driver deliveries, requests", stats.driver_meals_delivered + stats.driver_customers_delivered, num_requests) || error;
    check_equal("Driver count and request counts",
        stats.driver_meals_picked_up + stats.driver_customers_picked_up,
        stats.total_meals_picked_up + stats.total_customers_picked_up) || error;

    printf("Success!\n");
    return 0;
}
