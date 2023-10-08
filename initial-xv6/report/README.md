## Implementation of Scheduling Policies
### Round Robin
#### An Overview
This is the default scheduling policy for the XV-6 codebase. The time slice used for this implementation is 1 tick. The algorithm iterates through the list of all processes, allowing runnable processes to execute for the slice time befor moving on to the next runnable process.
#### Implementation
An inner loop iterates throuh the list of all processes. The outer loop runs infinitely. Thus, the list of processes are iterated infinitely in a circle (round-robin!). If the state of a process is RUNNABLE, it is run (set to RUNNING) for one tick, which causes a user interrupt. Here, the yield() function is called. The control is given back to the CPU and the context of the CPU is restored. After yield() returns, the scheduler gets back control and runs the next RUNNABLE process.
### First Come First Serve
#### An Overview

#### Implementation

### Multi-Level Feedback Queue
#### An Overview

#### Implementation

## Performance Comparison

`schedulertest` was run for the 3 scheduling algorithms multiple times and an averge of `rtime` and `wtime` were taken.
`CPUS` was set to 1. `NFORK` (number of CPU-bound processes) in the `schedulertest.c` file was set to 5 and `IO` (number of IO bound processes) was set to 0.

|         Scheduling policy         | Average run time | Average wait time |
| :-------------------------------: | :--------------: | :---------------: |
|          Round Robin (RR)         |        10        |        142        |
|   First Come First Serve (FCFS)   |        10        |        120        |
| Multi-Level Feedback Queue (MLFQ) |        10        |        134        |

The average run time for all the scheduling policies comes around to be the same as the same number of CPU-bound processes are being executed, thus, the ticks required to execute these processes also remains the same.

The average wait time however, appears to be in the increasing order of
- FCFS
- MLFQ
- RR

This can be explained by the fact that during FCFS, the yield() function is never called in usertrap(). Thus we never yield the CPU except for kernel traps. As all the processes are of the same length (forked from the same schedulertest process, running in a loop from 0 to 1e9 ), FCFS has the least waiting time for equal size processes because of the overhead that arises because of context-switching is minimized by not yielding control to run another process. However this policy does not take into account, fairness. Besides equally long processes, none of the processes perform I/O, in which case MLFQ would have been faster as it switches to other RUNNABLE processes when the currenly RUNNING process starts I/O.

MLFQ ranks the second, over RR, as it does not call yield() for each tick like Round-Robin does, but only after the slice-time of the queue the currently running process is in. So, enen though the slice-time is initially 1 and yield() is called on userinterrupt for each tick, for a process 3rd priority queue, yield() only occurs after the process runs for 9 ticks, thus reducing context switch overhead compared to RR, but not as much as FCFS.

Results would vary depending on having more I/O bound processes and processes of different lengths, where MLFQ, and even RR may have been more efficient than FCFS.

## MLFQ Scheduling Analysis
### Timeline plot between queue number on the Y-axis and ticks elapsed on the X-axis for 5 proccesses
![Alt text](plot.png)
##### Note: The CPU process loop was run for 1.8e9 instead of 1e9 to properly display scheduling
In the MLFQ scheduling graph, processes are represented as they move between different priority queues. As processes enter low-priority queues, they rise upwards on the y-axis of the graph, indicating their transition to lower priority. This happens when a process has consumed its entire time slice in the current queue. Conversely, processes descend on the y-axis when they enter higher-priority queues, which occurs after they exceed the AGE time in their current queue, as determined by the aging function. This representation captures the scheduling behavior of processes as they compete for CPU time across multiple priority levels in the MLFQ scheduler.


