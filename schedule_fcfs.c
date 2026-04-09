/*
 * Implementation of FCFS scheduling algorithm.
 * Team Member 1: Diana Gonzalez
 * Q1 - Run tasks in FCFS order using schedule_arrive_zero.txt
 *      All tasks arrive at time 0, so the CPU is never idle.
 *      Tasks are ordered purely by name (T1 before T2, etc.)
 * Q2 - Print process metrics table:
 *        - Turnaround Time  = finish_time - arrival_time
 *        - Waiting Time     = start_time  - arrival_time
 *        - Response Time    = start_time  - arrival_time
 *        - Response Ratio   = (waiting + burst) / burst
 * Q3 - Print average metrics:
 *        - Average Turnaround Time
 *        - Average Waiting Time
 *        - Throughput = tasks / total elapsed time
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "task.h"
#include "list.h"
#include "cpu.h"

// reference to the head of the list
struct node *head = NULL;

// sequence counter of next available thread identifier
int nextTid = 0;

Task *selectNextTask();

// add a new task to the list of tasks
void add(char *name, int arrivalTime, int burst) {
    // first create the new task
    Task *newTask = (Task *) malloc(sizeof(Task));

    newTask->name = name;
    newTask->tid = nextTid++;
    newTask->arrivalTime = arrivalTime;
    newTask->burst = burst;

    // insert the new task into the list of tasks
    insert(&head, newTask);
}


/* ---------------------------------------------------------------
 * selectNextTask()
 *
 * FCFS algorithm:
 *   1. pick the task with the SMALLEST arrivalTime (all are 0 so no arrival filtering)
 *   2. if more than one share arrivalTime - pick the next task alphabetically
 * --------------------------------------------------------------- */
Task *selectNextTask() {
    //initialized pointer 'ptr' to travel through all
    struct node *ptr = head;
    //assume the first node is the best
    Task *best = head->task;
    
    //keeps checking under there are no more nodes to check
    while (ptr != NULL) {
        //helps avoid writing ptr->task->arrivalTime 
        Task *t = ptr->task;

	//checks if different t->arrivalTime is smaller than best
        if (t->arrivalTime < best->arrivalTime ||
            //if same arrivalTime, best is the task that comes alphabetically first
            (t->arrivalTime == best->arrivalTime &&
             strcmp(t->name, best->name) < 0)) {
            best = t;
            //otherwise best remains same and checks next node
        }
        ptr = ptr->next;
    }
    return best;
}
