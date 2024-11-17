//*******************************************************************
// Name: Tanishq Patil
// Red ID: 132639686
// Assignment 2: Implementing Shortest Job first logic for scheduling
// queue for storing pointers to processes
//*******************************************************************
#ifndef QUEUE_H
#define QUEUE_H

#include<stdlib.h>
#include<stdio.h>
#include<unistd.h>


// Implementing a simple queue data structure using a linked list
/**
 * @brief Node stores a pointer to the process
 * 
 */
typedef struct Node{
    int req_type;
    struct Node* next;
}Node;

typedef struct queue{
    Node* head;
    Node* tail;
}queue_t;

/** @brief Create a new Node
   
   @return a pointer to newly created node
*/
Node* create_node(int data);

/** @brief Dynamically allocates and initializes a new queue 
 *
 *
 *  @return a pointer to a new queue_t
 */
queue_t *create_queue();


/** @brief push an element onto a queue
 *
 *  @param q the queue to push an element into.
 *
 *  @param elem th element to add to the queue
 *
 *  @return A bool indicating success or failure.  Note, the function
 *          should succeed unless the q parameter is NULL.
 */
void push_to_queue(queue_t *q, int data);

/** @brief pop an element from a queue.
 *
 *  @param q the queue to pop an element from.
 *
 *  @param elem a place to assign the poped element.
 *
 *  @return A bool indicating success or failure.  Note, the function
 *          should succeed unless the q parameter is NULL.
 */

int pop_queue(queue_t *q);

/** @brief Delete your queue and free all of its memory.
 *
 *  @param q the queue to be deleted.  Note, you should assign the
 *  passed in pointer to NULL when returning (i.e., you should set
 *  *q = NULL after deallocation).
 *
 */
void delete_queue(queue_t *q);


#endif