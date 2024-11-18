
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
#define DENOM 1000


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
        //printf("General about to sleep for %d\n", sync_monitor->general_sleep);
        //fflush(stdout);
        sleep(sync_monitor->general_sleep/DENOM); // general request simulation
        //printf("production complete in: %d milisec\n", sync_monitor->general_sleep);
        //fflush(stdout);
        //acquire lock
        //printf("production complete\n");
        pthread_mutex_lock(&sync_monitor->lock);
        //printf("lock acquired by general\n");
        //fflush(stdout);
        //printf("lock acquired\n");
        //First check if the number requests are less than the maximum allowed requests
        if(sync_monitor->request_count == sync_monitor->max_requests-1){
            //unlock
    //        printf("general is leaving\n");
            pthread_mutex_unlock(&sync_monitor->lock);
            //printf("lock released by general, general is leaving\n");
            //fflush(stdout);
            // if the requests produced has reached its limit, signal the main thread
            // wake up all threads
            printf("general will signal consumers waiting on empty to move on\n");
            fflush(stdout);
            pthread_cond_signal(&sync_monitor->empty);
            sem_post(sync_monitor->barrier_gen);
            return NULL;
        }
        else{
            //printf("general should check if buffer is available\n");
            //fflush(stdout);
            //printf("Queue size: %d\n", sync_monitor->queue_size);
            //fflush(stdout);
            // check if the queue is full
            while(sync_monitor->queue_size == MAX_QUEUE_SIZE){

          //      sync_monitor->queue_full_flag = FULL;// queue is full
                // wait if the queue is full, once a spot opens acquire the lock
    //            printf("general is waiting for queue space\n");
                //printf("Queue is full, general is waiting\n");
                //fflush(stdout);
                pthread_cond_wait(&sync_monitor->full, &sync_monitor->lock);
                
            }
            //printf("queue is available for general to add request\n");
            //fflush(stdout);
            // add the request to the queue
            push_to_queue(sync_monitor->wait_queue, GEN_REQ); // we will use 0 to denote a general request
            // update the requests count in the queue
            sync_monitor->requests_count_arr[GEN_REQ] += 1;
            //update total produced
            sync_monitor->total_requests_prod[GEN_REQ] +=1;
            //increment the queue counter by 1
            //old queue size
            //int old_size = sync_monitor->queue_size;
            sync_monitor->queue_size+=1;
            sync_monitor->request_count +=1; // increase the number of requests count by 1

            //printf("general request succesfully added\n");
            //fflush(stdout);
            //printf("queue: %d\n", sync_monitor->queue_size);
            //fflush(stdout);
            // use the log function to write to stdout
            output_request_added(type, sync_monitor->total_requests_prod, sync_monitor->requests_count_arr);
             // Signal consumers if the queue was empty, we added an element
            if (sync_monitor->queue_size == 1) {
                //  printf("queue is no longer empty, signal the consumers\n");
                //  fflush(stdout);
                pthread_cond_signal(&sync_monitor->empty);
            }
           // printf("general is leaving the lock\n");
           // fflush(stdout);
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
        //printf("vip about to sleep for : %d\n", sync_monitor->vip_sleep);
        //fflush(stdout);
        //produce a request, simulate by sleeping
        sleep(sync_monitor->vip_sleep/DENOM); // VIP request simulation
        //printf("VIP completed its sleep\n");
        //fflush(stdout);
        //acquire lock
        pthread_mutex_lock(&sync_monitor->lock);
        //printf("lock acquired by VIP\n");
        //fflush(stdout);
        //First check if the number requests are less than the maximum allowed requests
        //printf("lock with vip\n");
        if(sync_monitor->request_count == sync_monitor->max_requests-1){
            //unlock
        //    printf("VIP leaving\n");
        //    printf("max requests have been met, VIP leaving\n");
        //    fflush(stdout);
            pthread_mutex_unlock(&sync_monitor->lock);
            printf("VIP has released the lock\n");
            fflush(stdout);
            // wake up all threads who are waiting
            pthread_cond_signal(&sync_monitor->empty);

            // if the requests produced has reached its limit, signal the main thread
            sem_post(sync_monitor->barrier_vip);
            return NULL;
        }
        else{
        //    printf("VIP should be added to queue, still posseses lock\n");
        //    fflush(stdout);
        //    printf("check the buffer: %d\n", sync_monitor->queue_size);
        //    fflush(stdout);

            // check if the queue is full
            while((sync_monitor->queue_size == MAX_QUEUE_SIZE) || (sync_monitor->requests_count_arr[VIP_REQ] == MAX_VIPS)){

        //        printf("VIP must wait fur buffer to free\n");
        //        fflush(stdout);              
                // wait if the queue is full, once a spot opens go ahead
                pthread_cond_wait(&sync_monitor->full, &sync_monitor->lock);
                
            }

            // printf("check the number of VIPs in the buffer: %d\n", sync_monitor->requests_count_arr[VIP_REQ]);
            // fflush(stdout);            
            // // check if the queue ahs maximum allowed vips
            // while(sync_monitor->requests_count_arr[VIP_REQ] == MAX_VIPS){
            //     //wait unitl signal
            //     printf("VIP must wait, too many vips in queue\n");
            //     fflush(stdout);
            //     pthread_cond_wait(&sync_monitor->vip_buf,&sync_monitor->lock);
            // }
        //    printf("verifying queue size: %d and num vips: %d\n", sync_monitor->queue_size, sync_monitor->requests_count_arr[VIP_REQ]);
        //    fflush(stdout);
 
            // add the request to the queue
            push_to_queue(sync_monitor->wait_queue, VIP_REQ);
            // update the requests count in the queue
            sync_monitor->requests_count_arr[VIP_REQ] += 1;
            //update total produced
            sync_monitor->total_requests_prod[VIP_REQ] +=1;
            //increment the queue counter by 1
            sync_monitor->queue_size+=1;
                       // add the vip request
            sync_monitor->request_count +=1; // increase the number of requests count by 1
        //    printf("Added the VIP request in queue\n");
        //    fflush(stdout);
            // use the log function to write to stdout
            output_request_added(type, sync_monitor->total_requests_prod, sync_monitor->requests_count_arr);

            if(sync_monitor->queue_size == 1){ // if we just added an elemnt to empty queue
                // signal the queue is not empty, we added an element
        //        printf("VIP signals that the queue is not empty\n");
        //        fflush(stdout);           
                pthread_cond_signal(&sync_monitor->empty);
            }
        //    printf("Release the lock, VIP\n");
        //    fflush(stdout);
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
        //printf("T_X has acquired the lock\n");
        //fflush(stdout);       
        // check if the queue is empty
        while(sync_monitor->queue_size == 0){
          //  printf("The queue is empty, check if more requests are expected\n");
            //fflush(stdout);        
            // check if production is complete
            if(sync_monitor->request_count == sync_monitor->max_requests-1){
                // the threads work is done so produce the output history message
                //printf("NO more requests expected\n");
              //  fflush(stdout);
                // if all the requests have been processed, signal the main
                //release the lock
                printf("T_X releasing the lock and signaling consumption done\n");
                fflush(stdout);
                // if other thread is waiting because queue is empty, let it know, TBI later
                pthread_cond_signal(&sync_monitor->empty);// let the other thread knwo if waititng
                pthread_mutex_unlock(&sync_monitor->lock);
                sem_post(sync_monitor->barrier_t_x); // signal barrier sem
                return NULL;
            }
            printf("T_X waiting for queue to populate\n");
            fflush(stdout);
            // wait till queue has an element
            pthread_cond_wait(&sync_monitor->empty, &sync_monitor->lock);
        }
        int req_typ = pop_queue(sync_monitor->wait_queue);// once lock is acquired above, fetch the request
    //    printf("Request from queue fetched by T_X\n");
    //    fflush(stdout);
        // request type
        RequestType req = req_typ;
        // decrease the queue size by 1
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
        // if the handled reques was vip and the buffer has a space open for vip
        if(req_typ == VIP_REQ && vip_ctr == MAX_VIPS){
        //     printf("VIP request was consumed by T_X, signal free vip space\n");
        //     fflush(stdout);

             pthread_cond_signal(&sync_monitor->full);
        }

        if(sync_monitor->queue_size == MAX_QUEUE_SIZE -1){
    //        printf("T_X is signaling the queue is not full\n");
    //        fflush(stdout);
            pthread_cond_signal(&sync_monitor->full);
        }
    //    printf("T_X is releasing the lock\n");
    ///    fflush(stdout);
        // release the lock
        pthread_mutex_unlock(&sync_monitor->lock);
        // check what kind of request
        // simulate
     //   printf("T_X sleeping for : %d\n", sync_monitor->t_x_sleep);
     //   fflush(stdout);
        sleep(sync_monitor->t_x_sleep/DENOM);
    //    printf("T_X woken up");
    //    fflush(stdout);

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
      //  pthread_mutex_lock(&sync_monitor->lock);
      ///  printf("Rev_9 acquired the lock\n");
      //  fflush(stdout);
        // check if the queue is empty
        while(sync_monitor->queue_size == 0){
    //        printf("Rev_9 says queue is empty, check if production is met\n");
    //        fflush(stdout);        
            // check if production is complete
            if(sync_monitor->request_count == sync_monitor->max_requests-1){
    //            printf("Rev_9 says production is met, rev_9 is leaving\n");
    //            fflush(stdout);
                // the threads work is done so produce the output history message

                // if all the requests have been processed, signal the main
    //            //release the lock
                printf("Rev_9 released the lock, and signaled main\n");
                fflush(stdout);
                pthread_cond_signal(&sync_monitor->empty);// let the other consumer know if waititng
                pthread_mutex_unlock(&sync_monitor->lock);
                sem_post(sync_monitor->barrier_rev_9); // signal barrier sem
                return NULL;
            }
            printf("Rev_9 is waiting for queue to populate\n");
            fflush(stdout);
            // wait till someone signals that the queue is not empty

            pthread_cond_wait(&sync_monitor->empty, &sync_monitor->lock);
        }
    //    printf("Rev_9 is found a request to handle, queue not empty\n");
    //    fflush(stdout);
        int req_typ = pop_queue(sync_monitor->wait_queue);// once lock is acquired above, fetch the request
        // request type
        RequestType req = req_typ;
        // decrease the queue size by 1
        //old queue size
        //int old = sync_monitor->queue_size;
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
        // if the popped value was vip and we have empty vip
        if(req_typ == VIP_REQ && vips == MAX_VIPS){
        //     printf("VIP request was consumed by rev_9, signal free vip space\n");
        //     fflush(stdout);

             pthread_cond_signal(&sync_monitor->full);
        }
        if(sync_monitor->queue_size == MAX_QUEUE_SIZE-1){
    //        printf("request was consumed by rev_9, signal queue is free\n");
    //        fflush(stdout);

            pthread_cond_signal(&sync_monitor->full);
        }
        // release the lock
    //    printf("rev_9 is releasing the lock\n");
    //    fflush(stdout);

        pthread_mutex_unlock(&sync_monitor->lock);
        // check what kind of request
        // simulate
    //    printf("rev_9 is sleeping\n");
        sleep(sync_monitor->rev_9_sleep/DENOM);
    //    printf("rev_9 is awake and ready\n");

    }          
}