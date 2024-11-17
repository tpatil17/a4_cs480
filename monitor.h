#ifndef MONITOR_H
#define MONITOR_H

#include<pthread.h>
#include<string.h>
#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<semaphore.h>
#include "queue.h"

typedef struct monitor {
    int max_requests; // the maximum allowed requests to produce
    int request_count; // number of requestes produced
    int queue_size; // number of guests in the queue, max allowed is 18
    unsigned int *requests_count_arr; //  array of the count of request types in the queue
    unsigned int **consumed_count_arr; // array of count of each type of request consumed (2d array indicating consumption for each consumer)
    pthread_cond_t full; // condition variable to signla buffer is full
    pthread_cond_t empty; // condition variable to signal buffer is empty
    queue_t* wait_queue; // bounded buffer
    pthread_mutex_t lock; // mutext lock to synchronize buffer access
    sem_t barrier_gen; // barrier semaphore for general greater
    sem_t barrier_vip; // barrier semaphore for general greater
    sem_t barrier_t_x; // barrier semaphore for general greater
    sem_t barrier_rev_9; // barrier semaphore for general greater
    int general_sleep; //sleep time for general request
    int vip_sleep; // sleep time for vip request
    int t_x_sleep; // sleep time for t-x bot
    int rev_9_sleep; // sleep time for rev_9 
    

}monitor;
/**
 * @brief Function to initialize monitor structure
 * 
 * @param total_request : the maximum requests to produce by the producer
 * @return monitor* : returns a pointer to the monitor struct
 */
monitor* init_monitor(int total_request);

/**
 * @brief The producer general function creates a general table request
 * waits for space to open up in the queue, and the adds the process to the queue
 * If the maximum number of production is met, the thread signals barrier semaphore
 * 
 * @param monitor : monitor struct to access wait queue and other sync tools
 */
void *producer_general(void* args);

/**
 * @brief This function simulates the consumption/processing a request for table
 * the function uses monitor to access the wait queue, and the condition variables in
 * monitor to check if the queue is empty, waits if empty, else acquires lock,
 * fetches the first item and then releases the lock. Sleeps for sleep time and 
 * waits for a new request. Once the queue is empty and the max requests are met,
 * the consumer thread signals main thread it has completed execution using the barrier_t_x
 * semaphore.
 * @param monitor : the monitor structure  used to synchronise threads
 */
void *consumer_t_x(void* args);

#endif