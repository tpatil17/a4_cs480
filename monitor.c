
#include<pthread.h>
#include<string.h>
#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<semaphore.h>
#include "queue.h"
#include "monitor.h"
#include<stdbool.h>
#include "log.h"
#define NORMAL_EXIT 0

monitor* init_monitor(int total_request){
    // initialize a monitor type variable
    monitor* monitor_t;
    // allocate memory to the monitor
    monitor_t = (monitor*)malloc(sizeof(monitor));
    // check if memory succesfully allocated
    if(monitor_t == NULL){
        printf("memory allocation for monitor failed\n");
        exit(NORMAL_EXIT); // normal exit
    }
    // initialize the max request to the total requests
    monitor_t->max_requests = total_request;
    // initialize request count to 0
    monitor_t->request_count = 0;
    // initialize the queue size to 0
    monitor_t->queue_size = 0;
    // initialize the array of number of requests in queue
    monitor_t->requests_count_arr = (unsigned int*)mallloc(sizeof(unsigned int)*2);
    // initialize the values to 0
    monitor_t->requests_count_arr[0] = 0; // general requests count 0
    monitor_t->requests_count_arr[1] = 0; // vip requests count 0
    // initialize the consumed type array 
    monitor_t->consumed_count_arr = (unsigned int**)malloc(sizeof(unsigned int)*2);
    //allocate memory for each consumer, t_x first
    monitor_t->consumed_count_arr[0] = (unsigned int *)malloc(sizeof(unsigned int)* 2);
    //allocate memory for each consumer, rev_9
    monitor_t->consumed_count_arr[1] = (unsigned int *)malloc(sizeof(unsigned int)* 2);
    // assign initial 0 values to each for t_x
    monitor_t->consumed_count_arr[0][0] = 0; //  general requests to 0
    monitor_t->consumed_count_arr[1][0] = 0; // vip requests to 0
    // for rev_9
    monitor_t->consumed_count_arr[1][0] = 0; //  general requests to 0
    monitor_t->consumed_count_arr[1][1] = 0; // vip requests to 0
    //allocate memory for the mutex lock
    pthread_mutex_init(&monitor_t->lock, NULL);
    // allocate memory for the condition variables
    pthread_cond_init(&monitor_t->full, NULL);
    pthread_cond_init(&monitor_t->empty, NULL);
    //allocate memory for the bounded buffer
    monitor_t->wait_queue = create_queue();
    // initialize barrier semaphores
    sem_init(&monitor_t->barrier_gen,0, 1);// barrier sem for general bot
    // for vip bot
    sem_init(&monitor_t->barrier_vip, 0, 1);
    // initialize barrier semaphore for t-x bot
    sem_init(&monitor_t->barrier_t_x,0, 1);
    // for rev_9
    sem_init(&monitor_t->barrier_rev_9, 0, 1);
    // default sleep time for general order
    monitor_t->general_sleep = 0; // default is 0
    // for t-x
    monitor_t->t_x_sleep = 0; 
    // for rev-9
    monitor_t->rev_9_sleep = 0;
    // for vip sleep
    monitor_t->vip_sleep = 0;
    
    return monitor_t;
}

void *producer_general(void *args){
    // type cast the monitor
    monitor* sync_monitor = (monitor*)args;
    // continue executing until production limit is met
    while(true){
        //First check if the number requests are less than the maximum allowed requests
        if(sync_monitor->request_count == sync_monitor->max_requests){
            //
            // if the requests produced has reached its limit, signal the main thread
            sem_post(sync_monitor->barrier_gen);
            return;
        }
        else{
            //produce a request, simulate by sleeping
            sleep(sync_monitor->general_sleep); // general request simulation
            sync_monitor->request_count +=1; // increase the number of requests count by 1

            // check if the queue is full
            if(sync_monitor->queue_size == 18){
                // wait if the queue is full, once a spot opens acquire the lock
                pthread_cond_wait(&sync_monitor->full, &sync_monitor->lock);
            }
            // add the request to the queue
            push_to_queue(sync_monitor->queue_size, 0); // we will use 0 to denote a general request
            // update the requests count in the queue
            sync_monitor->requests_count_arr[0] += 1;
            //increment the queue counter by 1
            sync_monitor->queue_size+=1;
            // use the log function to write to stdout
            output_request_added(producerNames[0], producerAbbrevs[0], sync_monitor->requests_count_arr);          
            // signal the queue is not empty
            pthread_cond_signal(&sync_monitor->empty);
            // leave the lock
            pthread_mutex_unlock(&sync_monitor->lock);

        }
    }
}

void *consumer_t_x(void *args){
    // type cast the args
    monitor* sync_monitor = (monitor*)args;
    // continue until no requests remain to be processed
    while(true){
        // check if the queue is empty
        while(sync_monitor->queue_size == 0){
            // check if production is complete
            if(sync_monitor->request_count == sync_monitor->max_requests){
                // the threads work is done so produce the output history message

                // if all the requests have been processed, signal the main
                sem_post(sync_monitor->barrier_t_x);
                return;
            }
            // wait to acquire lock and someone signals that the queue is not empty
            pthread_cond_wait(&sync_monitor->empty, &sync_monitor->lock);
        }
        int req_typ = pop_queue(sync_monitor->wait_queue);// once lock is acquired above, fetch the request
        // decrease the queue size by 1
        sync_monitor->queue_size-=1;
        // since we are processing the current request type
        // decrease its counter from shared variable
        sync_monitor->requests_count_arr[req_typ]-=1;
        // use the log library to log the output
        output_request_removed(consumerNames[0], producerNames[req_typ], 
        sync_monitor->consumed_count_arr[0][req_typ], sync_monitor->requests_count_arr[req_typ] 
        );
        //signal queue is not full
        pthread_cond_signal(&sync_monitor->full);
        // release the lock
        pthread_mutex_unlock(&sync_monitor->lock);
        // check what kind of request
        // simulate
        sleep(sync_monitor->t_x_sleep);

    }

}