
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
#define MAX_QUEUE_SIZE 18
#define GEN_REQ 0
#define VIP_REQ 1
#define T_X 0
#define REV_9 1
#define FULL 1
#define NOT_FULL 0
#define NOT_EMPTY 0
#define EMPTY 1
#define ROWS 2
#define MAX_VIPS 5


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
    monitor_t->requests_count_arr = (unsigned int*)malloc(sizeof(unsigned int)*ROWS);
    // initialize the values to 0
    monitor_t->requests_count_arr[GEN_REQ] = 0; // general requests count 0
    monitor_t->requests_count_arr[VIP_REQ] = 0; // vip requests count 0
    // initialize the consumed type array 
    monitor_t->consumed_count_arr = (unsigned int**)malloc(sizeof(unsigned int*)*ROWS);
    //allocate memory for each consumer, t_x first
    monitor_t->consumed_count_arr[T_X] = (unsigned int *)malloc(sizeof(unsigned int)* ROWS);
    //allocate memory for each consumer, rev_9
    monitor_t->consumed_count_arr[REV_9] = (unsigned int *)malloc(sizeof(unsigned int)* ROWS);
    // assign initial 0 values to each for t_x
    monitor_t->consumed_count_arr[T_X][GEN_REQ] = 0; //  general requests to 0
    monitor_t->consumed_count_arr[T_X][VIP_REQ] = 0; // vip requests to 0
    // for rev_9
    monitor_t->consumed_count_arr[REV_9][GEN_REQ] = 0; //  general requests to 0
    monitor_t->consumed_count_arr[REV_9][VIP_REQ] = 0; // vip requests to 0
    //allocate memory for the mutex lock
    pthread_mutex_init(&monitor_t->lock, NULL);
    // allocate memory for the condition variables
    pthread_cond_init(&monitor_t->full, NULL);
    pthread_cond_init(&monitor_t->empty, NULL);
    pthread_cond_init(&monitor_t->vip_buf, NULL);
    //allocate memory for the bounded buffer
    monitor_t->wait_queue = create_queue();
    // Allocate memory for the semaphore pointers
    monitor_t->barrier_gen = (sem_t *)malloc(sizeof(sem_t));
    monitor_t->barrier_vip = (sem_t *)malloc(sizeof(sem_t));
    monitor_t->barrier_t_x = (sem_t *)malloc(sizeof(sem_t));
    monitor_t->barrier_rev_9 = (sem_t *)malloc(sizeof(sem_t));
    // initialize barrier semaphores
    sem_init(monitor_t->barrier_gen,0, 0);// barrier sem for general bot
    // for vip bot
    sem_init(monitor_t->barrier_vip, 0, 0);
    // initialize barrier semaphore for t-x bot
    sem_init(monitor_t->barrier_t_x,0, 0);
    // for rev_9
    sem_init(monitor_t->barrier_rev_9, 0, 0);
    // default sleep time for general order
    monitor_t->general_sleep = 0; // default is 0
    // for t-x
    monitor_t->t_x_sleep = 0; 
    // for rev-9
    monitor_t->rev_9_sleep = 0;
    // for vip sleep
    monitor_t->vip_sleep = 0;
    // flag queue to empty
    monitor_t->queue_empty_flag = EMPTY; 
    // flag queue is not full
    monitor_t->queue_full_flag = NOT_FULL;

    // initialise the total requests produced
    monitor_t->total_requests_prod = (unsigned int *)malloc(sizeof(unsigned int)*ROWS);
    // initialze to 0 for each
    monitor_t->total_requests_prod[GEN_REQ] = 0;
    monitor_t->total_requests_prod[VIP_REQ] = 0;
    
    return monitor_t;
}

void *producer_general(void *args){
    // type cast the monitor
    monitor* sync_monitor = (monitor*)args;
    //Assign request type
    RequestType type = GEN_REQ;
    // continue executing until production limit is met
    while(true){
        //produce a request, simulate by sleeping
        sleep(sync_monitor->general_sleep); // general request simulation
        //acquire lock
        //printf("production complete\n");
        pthread_mutex_lock(&sync_monitor->lock);
        //printf("lock acquired\n");
        //First check if the number requests are less than the maximum allowed requests
        if(sync_monitor->request_count == sync_monitor->max_requests){
            //unlock
            printf("general is leaving\n");
            pthread_mutex_unlock(&sync_monitor->lock);
            // if the requests produced has reached its limit, signal the main thread
            // wake up all threads
            pthread_cond_signal(&sync_monitor->empty);
            sem_post(sync_monitor->barrier_gen);
            return NULL;
        }
        else{

            sync_monitor->request_count +=1; // increase the number of requests count by 1

            // check if the queue is full
            while(sync_monitor->queue_size == MAX_QUEUE_SIZE){

                sync_monitor->queue_full_flag = FULL;// queue is full
                // wait if the queue is full, once a spot opens acquire the lock
                printf("general is waiting for queue space\n");
                pthread_cond_wait(&sync_monitor->full, &sync_monitor->lock);
                
            }
            // add the request to the queue
            push_to_queue(sync_monitor->wait_queue, GEN_REQ); // we will use 0 to denote a general request
            // update the requests count in the queue
            sync_monitor->requests_count_arr[GEN_REQ] += 1;
            //update total produced
            sync_monitor->total_requests_prod[GEN_REQ] +=1;
            //increment the queue counter by 1
            //old queue size
            int old_size = sync_monitor->queue_size;
            sync_monitor->queue_size+=1;
            printf("queue size: %d\n", sync_monitor->queue_size);
            // use the log function to write to stdout
            output_request_added(type, sync_monitor->total_requests_prod, sync_monitor->requests_count_arr);

            //if(old_size == 0){ // if we just added an elemnt to empty queue
                // signal the queue is not empty, we added an element
                sync_monitor->queue_empty_flag = NOT_EMPTY;
                pthread_cond_signal(&sync_monitor->empty);
            //}

            // leave the lock
            pthread_mutex_unlock(&sync_monitor->lock);

        }
    }
}

void* producer_vip(void * args){
        // type cast the monitor
    monitor* sync_monitor = (monitor*)args;
    //Assign request type
    RequestType type = VIP_REQ;
    // continue executing until production limit is met
    while(true){
        //produce a request, simulate by sleeping
        sleep(sync_monitor->vip_sleep); // VIP request simulation
        //acquire lock
        pthread_mutex_lock(&sync_monitor->lock);
        //First check if the number requests are less than the maximum allowed requests
        //printf("lock with vip\n");
        if(sync_monitor->request_count == sync_monitor->max_requests){
            //unlock
            printf("VIP leaving\n");
            pthread_mutex_unlock(&sync_monitor->lock);
            // wake up all threads who are waiting
            pthread_cond_signal(&sync_monitor->empty);
            // if the requests produced has reached its limit, signal the main thread
            sem_post(sync_monitor->barrier_vip);
            return NULL;
        }
        else{
            // add the vip request
            sync_monitor->request_count +=1; // increase the number of requests count by 1
            // check if the queue is full
            while(sync_monitor->queue_size == MAX_QUEUE_SIZE){

                sync_monitor->queue_full_flag = FULL;// queue is full
                // wait if the queue is full, once a spot opens go ahead
                printf("VIP waiting queue full\n");
                pthread_cond_wait(&sync_monitor->full, &sync_monitor->lock);
                
            }
            // check if the queue ahs maximum allowed vips
            while(sync_monitor->requests_count_arr[VIP_REQ] == MAX_VIPS){
                //wait unitl signal
                printf("vip waiting, too many vips\n");
                pthread_cond_wait(&sync_monitor->vip_buf,&sync_monitor->lock);
            }
            // add the request to the queue
            push_to_queue(sync_monitor->wait_queue, VIP_REQ);
            // update the requests count in the queue
            sync_monitor->requests_count_arr[VIP_REQ] += 1;
            //update total produced
            sync_monitor->total_requests_prod[VIP_REQ] +=1;
            //increment the queue counter by 1
            // old size
            int old_size = sync_monitor->queue_size;
            sync_monitor->queue_size+=1;
            // use the log function to write to stdout
            output_request_added(type, sync_monitor->total_requests_prod, sync_monitor->requests_count_arr);

            //if(old_size == 0){ // if we just added an elemnt to empty queue
                // signal the queue is not empty, we added an element
                sync_monitor->queue_empty_flag = NOT_EMPTY;
                pthread_cond_signal(&sync_monitor->empty);
            //}

            // leave the lock
            pthread_mutex_unlock(&sync_monitor->lock);

        }
    }
}

void *consumer_t_x(void *args){
    // type cast the args
    monitor* sync_monitor = (monitor*)args;
    // create a consumer type variable
    ConsumerType type = T_X; // assign type
    // continue until no requests remain to be processed
    while(true){
        // acquire lock
        pthread_mutex_lock(&sync_monitor->lock);
        // check if the queue is empty
        while(sync_monitor->queue_size == 0){
            // check if production is complete
            if(sync_monitor->request_count == sync_monitor->max_requests){
                // the threads work is done so produce the output history message

                // if all the requests have been processed, signal the main
                //release the lock
                pthread_mutex_unlock(&sync_monitor->lock);
                sem_post(sync_monitor->barrier_t_x); // signal barrier sem
                return NULL;
            }
            // flag the queue is empty
            sync_monitor->queue_empty_flag = EMPTY;
            // wait till someone signals that the queue is not empty
            printf("t-x is waiting\n");
            pthread_cond_wait(&sync_monitor->empty, &sync_monitor->lock);
        }
        int req_typ = pop_queue(sync_monitor->wait_queue);// once lock is acquired above, fetch the request
        // request type
        RequestType req = req_typ;
        // decrease the queue size by 1
        //old queue size
        int old_size = sync_monitor->queue_size;
        sync_monitor->queue_size-=1;
        // since we are processing the current request type
        // store the value of vips in queue prior to current process
        int vip_ctr = sync_monitor->requests_count_arr[VIP_REQ];
        // decrease the current request types ctr
        sync_monitor->requests_count_arr[req_typ]-=1;
        // increase the consumed count
        sync_monitor->consumed_count_arr[T_X][req_typ]+=1;
        // use the log library to log the output
        output_request_removed(type, req, 
        sync_monitor->consumed_count_arr[T_X], sync_monitor->requests_count_arr 
        );
        //signal queue is not full
        //if(old_size == MAX_QUEUE_SIZE){
            // set flag to not full as we just cleared an element
            sync_monitor->queue_full_flag = NOT_FULL;
            //signal the queue is not full anymore
            printf("queue no longer full\n");
            pthread_cond_signal(&sync_monitor->full);
        //}
        // if the handled reques was vip and the buffer has a space open for vip
        if(req_typ == VIP_REQ && vip_ctr == MAX_VIPS){
            printf("vip good to add\n");
            pthread_cond_signal(&sync_monitor->vip_buf);
        }
     
        // release the lock
        pthread_mutex_unlock(&sync_monitor->lock);
        // check what kind of request
        // simulate
        sleep(sync_monitor->t_x_sleep);

    }

}

void* consumer_rev_9(void* args){
    // type cast args
    monitor* sync_monitor = (monitor*)args;
    // create a consumer type variable
    ConsumerType type = REV_9; // assign type
    // continue until no requests remain to be processed
    while(true){
        // acquire lock
        pthread_mutex_lock(&sync_monitor->lock);
        // check if the queue is empty
        while(sync_monitor->queue_size == 0){
            // check if production is complete
            if(sync_monitor->request_count == sync_monitor->max_requests){
                // the threads work is done so produce the output history message

                // if all the requests have been processed, signal the main
                //release the lock
                pthread_mutex_unlock(&sync_monitor->lock);
                sem_post(sync_monitor->barrier_rev_9); // signal barrier sem
                return NULL;
            }
            // flag the queue is empty
            sync_monitor->queue_empty_flag = EMPTY;
            // wait till someone signals that the queue is not empty
            printf("rev_9 is waiting\n");
            pthread_cond_wait(&sync_monitor->empty, &sync_monitor->lock);
        }
        int req_typ = pop_queue(sync_monitor->wait_queue);// once lock is acquired above, fetch the request
        // request type
        RequestType req = req_typ;
        // decrease the queue size by 1
        //old queue size
        int old = sync_monitor->queue_size;
        sync_monitor->queue_size-=1;
        // since we are processing the current request type
        // capture current number of vips in queue
        int vips = sync_monitor->requests_count_arr[VIP_REQ];
        // decrease its counter from shared variable
        sync_monitor->requests_count_arr[req_typ]-=1;
        // increase the consumed count
        sync_monitor->consumed_count_arr[REV_9][req_typ]+=1;
        // use the log library to log the output
        output_request_removed(type, req, 
        sync_monitor->consumed_count_arr[REV_9], sync_monitor->requests_count_arr 
        );
        //signal queue is not full
       //if(old == MAX_QUEUE_SIZE){
            // set flag to not full as we just cleared an element
            sync_monitor->queue_full_flag = NOT_FULL;
            //signal the queue is not full anymore
            printf("queue no longer full\n");
            pthread_cond_signal(&sync_monitor->full);
        //}
        // if the request handeled was vip signal vip, and the queue has space for vip
        if(req_typ == VIP_REQ && vips == MAX_VIPS){
            printf("vip good to be added\n");
            pthread_cond_signal(&sync_monitor->vip_buf);
        }
        // release the lock
        pthread_mutex_unlock(&sync_monitor->lock);
        // check what kind of request
        // simulate
        sleep(sync_monitor->rev_9_sleep);

    }          
}