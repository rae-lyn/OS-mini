/*
 * Implementation of FCFS scheduling algorithm.
 * Team Member 1: Diana Gonzalez
 * Q1 - Run tasks in FCFS order using schedule_arrive_zero.txt
 *      All tasks arrive at time 0, so the CPU is never idle.
 *      Tasks are ordered purely by name (T1 before T2, etc.)
 * Q2 - Print process metrics table:
 *        - Turnaround Time  = completion time - arrival time
 *        - Waiting Time     = turnaround time - burst time
 *        - Response Time    = first CPU time  - arrival time
 *        - Response Ratio   = (waiting + burst) / burst
 * Q3 - Print average metrics:
 *        - Average Turnaround Time
 *        - Average Waiting Time
 *        - Throughput = tasks / completion time
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
 * FCFS algorithm:
 *   1. pick the task with the SMALLEST arrivalTime (all are 0 so no arrival filtering)
 *   2. if more than one share arrivalTime - pick the next task alphabetically
 * --------------------------------------------------------------- */
Task *selectNextTask(int current_time) {
    //initialized pointer 'ptr' to travel through all
    struct node *ptr  = head;
	//best candidate found so far
    Task *best = head->task;
    
    //keeps checking under there are no more nodes to check
    while (ptr != NULL) {
        //helps avoid writing ptr->task->arrivalTime 
        Task *t = ptr->task;

		//only consider this task if it has already arrived
        if (t->arrivalTime <= current_time) {
            if (t->arrivalTime < best->arrivalTime) {
                //this task arrived earlier than our current best — prefer it
                best = t;
            }
            else if (t->arrivalTime == best->arrivalTime &&
                     strcmp(t->name, best->name) < 0) {
                //ame arrival time — pick whichever name comes first alphabetically
                //strcmp returns negative if t->name comes before best->name
                best = t;
            }
        }
        ptr = ptr->next; //move to next node
    }
    return best;
}

/* ---------------------------------------------------------------
 * Simulates a non-preemptive FCFS CPU.
 * Tracks current_time so tasks that arrive after an idle CPU
 * are handled correctly.
 * Collects per-task metrics and prints summary tables.
 * --------------------------------------------------------------- */
void schedule() {
    /* shows ordered list first */
    printf("=== Tasks ===\n");
    traverse(head); //prints every node: [name] [arrivalTime] [burst]
    printf("\n");

    // count total tasks
    int n = 0;
    struct node *tmp = head;
    while (tmp != NULL) {
        n++; 
        tmp = tmp->next; 
    }

    // arrays to store metrics in execution order
    char  **names      = malloc(n * sizeof(char *));
    int   *arrival     = malloc(n * sizeof(int)); //all zero
    int   *burst_arr   = malloc(n * sizeof(int));
    int   *start_time  = malloc(n * sizeof(int));
    int   *finish_time = malloc(n * sizeof(int));
    int   *turnaround  = malloc(n * sizeof(int)); //completion - arrival
    int   *waiting     = malloc(n * sizeof(int)); //turnaround - burst
    int   *response    = malloc(n * sizeof(int)); //first CPU time - arrival
    float *resp_ratio  = malloc(n * sizeof(float)); // (waiting + burst) / burst

    int current_time = 0; //simulated clock
    int idx = 0; //metric array index

    //main scheduling loop
    while (head != NULL) {
        //gets next task from selectNextTask
        Task *current = selectNextTask(current_time);

        if (current == NULL) {
    		// No task has arrived yet — the CPU is idle.
    		// Find the earliest arrivalTime still in the list and
    		// jump the clock forward to that moment.
    		struct node *ptr  = head;
    		int earliest = ptr->task->arrivalTime;

    		while (ptr != NULL) {
        		if (ptr->task->arrivalTime < earliest) {
            		earliest = ptr->task->arrivalTime;
        		}
        		ptr = ptr->next;
    		}
    		// Jump the clock to when the next task arrives
    		current_time = earliest;
    		continue;  // loop back and try selectNextTask() again
		}

		// Record the first task's arrival time for throughput calculation
        if (first_arrival == -1) {
            first_arrival = current->arrivalTime;
        }
		
        //task starts (no idle time since all tasks arrive at 0)
        int t_start  = current_time;
        //task finishes after running its full burst duration
        int t_finish = current_time + current->burst;

        //simulate running task on CPU
        run(current, current->burst);
        //simulated clock moved to when task finished
        current_time = t_finish;

        //compute metrics
        int   ta = t_finish - current->arrivalTime; //turnaround = completion - arrival
        int   wt = ta - current->burst; //waiting = turnaround - burst
        int   rt = t_start  - current->arrivalTime; //response = first CPU time - arrival
        float rr = (float)(wt + current->burst) / current->burst; //resp ratio = (wait + burst) / burst

        //store all metrics for this task at position idx
        names[idx]       = current->name;
        arrival[idx]     = current->arrivalTime;
        burst_arr[idx]   = current->burst;
        start_time[idx]  = t_start;
        finish_time[idx] = t_finish;
        turnaround[idx]  = ta;
        waiting[idx]     = wt;
        response[idx]    = rt;
        resp_ratio[idx]  = rr;
        idx++;

        //remove this task from the linked list now that its done
        delete(&head, current);
    }

    //=== Q2: Per-Process Metrics Table ===
    printf("\n=== Q2: Per-Process Metrics ===\n");
    //print headers of column
    printf("%-6s %8s %6s %8s %10s %12s %10s %10s %14s\n",
           "Task", "Arrival", "Burst", "Start", "Finish",
           "Turnaround", "Waiting", "Response", "Resp.Ratio");
    //print a divider line under headers
    printf("%-6s %8s %6s %8s %10s %12s %10s %10s %14s\n",
           "------","--------","------","--------","----------",
           "------------","--------","--------","--------------");
    //accumulators for computing averages in Q3
    long total_ta = 0, total_wt = 0;

    //print one row per task and add to running totals
    for (int i = 0; i < n; i++) {
        printf("%-6s %8d %6d %8d %10d %12d %10d %10d %14.4f\n",
               names[i], arrival[i], burst_arr[i],
               start_time[i], finish_time[i],
               turnaround[i], waiting[i], response[i], resp_ratio[i]);
        total_ta += turnaround[i];
        total_wt += waiting[i];
    }

    //=== Q3: Average / Summary Metrics ===
    double avg_ta = (double)total_ta / n; //avg turnaround = total_ta / n
    double avg_wt = (double)total_wt / n; //avg waiting = total_wt / n
    double throughput = (double)n / (finish_time[n-1] - first_arrival); //throughput = n / completion time

    printf("\n=== Q3: Average / Summary Metrics ===\n");
    printf("%-30s %10.4f\n", "Average Turnaround Time:", avg_ta);
    printf("%-30s %10.4f\n", "Average Waiting Time:",   avg_wt);
    printf("%-30s %10.6f  (tasks/time unit)\n", "Throughput:", throughput);

    //free every array we allocated with malloc to avoid memory leaks
    free(names); free(arrival); free(burst_arr);
    free(start_time); free(finish_time);
    free(turnaround); free(waiting);
    free(response); free(resp_ratio);
}
