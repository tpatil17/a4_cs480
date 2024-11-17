#include<stdio.h>
#include<stdlib.h>
#include<getopt.h>
#include<string.h>
#include<pthread.h>
#include<unistd.h>
#include<semaphore.h>
#include "monitor.h"
#include "log.h"
#define NORMAL_EXIT 0
#define DENOM 1000

int main(int argc, char* argv[]){

    int option = 0;
    int max_requests = 120; //default maximum requests
    int t_x_time = 0; //time taken by t_x to assign a seat
    int rev_9_time = 0; // time taken by rev-9 to assign a seat
    int gen_time = 0; // time taken to generate a general seat request
    int vip_time = 0; // time taken to generate a vip seat request



    
    while ((option = getopt(argc, argv, "s:x:r:g:v:")) != -1) { // identify if alpha is specified through command line
        switch (option) {
            case 's':
                max_requests = strtof(optarg, NULL);  // set the maximum numbers of requests to entertain
                break;
            case 'x': // set the time taken by t-x robot to resolve a request
                t_x_time = strtof(optarg, NULL); 
                break;
            case 'r': // set the time taken by rev 9 robot
                rev_9_time = strtof(optarg, NULL); 
                break;
            case 'g':// set the time taken to generate a general seat request
                gen_time = strtof(optarg, NULL); 
                break;
            case 'v':// set the time taken to generate a vip seat request
                vip_time = strtof(optarg, NULL); 
                break;    
            default:
                break;
        }
    }
    // Debug statements for the get opt functionality

    
    printf("The total number of requests to be entertained: %d\n", max_requests);
    printf("The time taken by t-x robot to resplve a request: %d\n", t_x_time);
    printf("The time taken by rev-9 robot to resolve a request: %d\n", rev_9_time);
    printf("The time taken by general robot tot create a request: %d\n", gen_time);
    printf("The time taken by vip robot tot create a request: %d\n", vip_time);

    // create a monitor sturct to use 
    monitor* syn_monitor = init_monitor(max_requests);

    //asign sleep times
    syn_monitor->general_sleep = gen_time/DENOM; // convert time to miliseconds
    syn_monitor->t_x_sleep = t_x_time/DENOM;
    syn_monitor->vip_sleep = vip_time/DENOM;
    syn_monitor->rev_9_sleep = rev_9_time/DENOM;
    

    pthread_t general_greeter; // bot that greets/produces general seat members
    // create the generla producer thread and initialise it
    // call the producer function to simulate
    pthread_create(&general_greeter, NULL, producer_general, syn_monitor);
    // create a producer thread for VIP rooms
    pthread_t vip_greeter;
    // thread should call the producer vip function
    pthread_create(&vip_greeter, NULL,producer_vip, syn_monitor);
    // create a consumre thread
    pthread_t t_x; // T-x bot 
    // create a consumer t-x thread to consume requestsd
    // call the  consumer_t_x function 
    pthread_create(&t_x, NULL, consumer_t_x, syn_monitor);
    // create a new thread for rev-9
    pthread_t rev_9;
    pthread_create(&rev_9, NULL, consumer_rev_9, syn_monitor);

    // wait signal barrier for general producer and wait on it
    sem_wait(syn_monitor->barrier_gen);
    sem_wait(syn_monitor->barrier_vip);
    // wait on the consumer t-x to fininsh
    sem_wait(syn_monitor->barrier_t_x);
    sem_wait(syn_monitor->barrier_rev_9);

    //print the consumption history report
    output_production_history(syn_monitor->total_requests_prod, syn_monitor->consumed_count_arr);




    return 0;

}