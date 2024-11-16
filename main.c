#include<stdio.h>
#include<stdlib.h>
#include<getopt.h>
#include<string.h>
#include<pthread.h>
#include<unistd.h>
#define NORMAL_EXIT 0

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

    return 0;

}