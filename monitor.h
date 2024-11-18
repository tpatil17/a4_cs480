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
    unsigned int *total_requests_prod; // the total requests of each type produced till now
    unsigned int **consumed_count_arr; // array of count of each type of request consumed (2d array indicating consumption for each consumer)
    pthread_cond_t full; // condition variable to signla buffer is full
    pthread_cond_t empty; // condition variable to signal buffer is empty
    pthread_cond_t vip_buf; // conditon variable to signal vip buffer is awailable
    queue_t* wait_queue; // bounded buffer
    pthread_mutex_t lock; // mutext lock to synchronize buffer access
    sem_t* barrier_gen; // barrier semaphore for general greater
    sem_t* barrier_vip; // barrier semaphore for general greater
    sem_t* barrier_t_x; // barrier semaphore for general greater
    sem_t* barrier_rev_9; // barrier semaphore for general greater
    int general_sleep; //sleep time for general request
    int vip_sleep; // sleep time for vip request
    int t_x_sleep; // sleep time for t-x bot
    int rev_9_sleep; // sleep time for rev_9 
    int queue_empty_flag; // flag to know if queue is empty or not
    int queue_full_flag; // flag to know if queue is full or not
    

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
 * @param args : monitor struct to access wait queue and other sync tools
 */
void *producer_general(void* args);

/**
 * @brief Producer function for the vip greater, the function will simulate 
 * VIP requests. Will be called by the vip greeter. args will be a pointer to monitor
 * struct. The function will create a VIP request and add to the queue.
 * 
 * @param args monitor struct
 * @return void* NULL
 */
void *producer_vip(void* args);
/**
 * @brief This function simulates the consumption/processing a request for table
 * the function uses monitor to access the wait queue, and the condition variables in
 * monitor to check if the queue is empty, waits if empty, else acquires lock,
 * fetches the first item and then releases the lock. Sleeps for sleep time and 
 * waits for a new request. Once the queue is empty and the max requests are met,
 * the consumer thread signals main thread it has completed execution using the barrier_t_x
 * semaphore.
 * @param args : the monitor structure  used to synchronise threads
 */
void *consumer_t_x(void* args);

/**
 * @brief function for the consumer rev_9. This function will be called by the rev_9 thread
 * to simulate catering to a request by rev_9. The argument is a pointer to the monitor data structure
 * using monitor the consumer will synchronize with other thread to consume requests from the buffer
 * 
 * @param args  : monitor struct
 * @return void* 
 */
void *consumer_rev_9(void* args);

#endif