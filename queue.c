//*******************************************************************
// Name: Tanishq Patil
// Red ID: 132639686
// Assignment 2: Implementing Shortest Job first logic for scheduling
// source code for queue.h
//*******************************************************************
#include"queue.h"
#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<string.h>


#define NORMAL_EXIT 0

// initialize node struct and allocates memory
Node* create_node(int data){
    Node* new_node = (Node*)malloc(sizeof(Node));

    if(!new_node){
        printf("Failed to allocate memory for a new node\n");
        exit(NORMAL_EXIT);
    }

    new_node->req_type = data;
    new_node->next = NULL;

    return new_node;

}

// creates a queue by allocating memory and returning a pointer to the queue
queue_t* create_queue(){

    queue_t* new_queue = (queue_t*)malloc(sizeof(queue_t));

    if(!new_queue){
        printf("Failed to allocate memory for a new queue\n");
        exit(NORMAL_EXIT);
    }

    new_queue->head = NULL;
    new_queue->tail = NULL;

    return new_queue;

}

// function used to push nodes to a queue, the nodes are pushed to the back of a queue
void push_to_queue(queue_t *q, int data){

    Node* new_node = create_node(data);

    if(q->tail == NULL){
        q->head = new_node;
        q->tail = new_node;
    }
    else{
        q->tail->next = new_node;
        q->tail = new_node;
    }

    return;

}

// pop returns the data stored inside the node pointed by queue head
// in this case a pointer to process is returned
int pop_queue(queue_t *q){

    if(q->head == NULL){
        printf("Empty queue cannot be popped\n");
        exit(NORMAL_EXIT);
    }

    int ret_data = q->head->req_type;

    Node *temp = q->head;

    q->head = q->head->next;

    if(q->head == NULL){
        q->tail = NULL;
    }

    free(temp);

    return ret_data;
    
}
// freeing allocated memory
void delete_queue(queue_t *q){
    while(q->head != NULL){
        pop_queue(q);
    }
    free(q);
}