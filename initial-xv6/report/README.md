## Implementation of Scheduling Policies
Use `make qemu SCHEDULER=X CPUS=Y` to specify which scheuler and how many CPUs to use. X can take the values `MLFQ`, `FCFS` and `ROUNDROBIN`. By default, `ROUNDROBIN` is executed.

### Round Robin
#### An Overview
This is the default scheduling policy for the XV-6 codebase. The time slice used for this implementation is 1 tick. The algorithm iterates through the list of all processes, allowing runnable processes to execute for the slice time befor moving on to the next runnable process.
#### Implementation
An inner loop iterates throuh the list of all processes. The outer loop runs infinitely. Thus, the list of processes are iterated infinitely in a circle (round-robin!). If the state of a process is RUNNABLE, it is run (set to RUNNING) for one tick, which causes a user interrupt. Here, the yield() function is called. The control is given back to the CPU and the context of the CPU is restored. After yield() returns, the scheduler gets back control and runs the next RUNNABLE process.
### First Come First Serve
#### An Overview
FCFS operates on a straightforward principle: the process that appears first in the process list will be executed first and will continue running until its completion. Subsequent processes that arrive after the initial process must patiently wait until the earlier arrived process finishes its execution. This scheduling approach ensures that processes are serviced in the order of their arrival, adhering to the "first come and first serve" rule. Processes arriving at the same time are serviced according to which process appears first on iterating over all the processes.

#### Implementation
We iterate throuh the list of all processes. The process with a RUNNABLE state with the earliest creation time, as indicated by the ctime variable in the proc struct, is selected. This chosen process is then assigned to the minProc variable, which is of type struct proc* and initially set to 0. If a valid process is successfully selected (i.e., when minProc is non-zero), the CPU is allocated to the process pointed to by minProc (we set the process state to RUNNING). This allocation is achieved by invoking the swtch function to perform a context switch. yield() is never called in usertrap() So that the RUNNABLE process, once selected by the scheduler is run until its completion and only then, the next earliest process is picked by the scheduler to run. This runs infinitely for the list of processes with an infinite outer loop.
This implementation ensures that the CPU time is granted to the process with the earliest creation time, adhering to the "First-Come, First-Served (FCFS)" principle, where the process that arrived first is served first.
### Multi-Level Feedback Queue
#### An Overview
The MLFQ (Multi-Level Feedback Queue) scheduling policy employs a priority queue system consisting of four levels, denoted from 0 to 3, where queue 0 represents the highest priority, and queue 4 represents the lowest. When a new process enters the system, it is initially placed in the highest priority queue (queue 0). A process in a given queue is run for a predetermined time-slice of:
- Queue 0 : 1 tick
- Queue 1: 3 ticks
- Queue 2: 9 ticks
- Queue 3: 15 ticks
As a process runs, it consumes its allocated time-slice. If the process completes its execution within this given time-slice, it  remains in the same queue. However, if the process doesn't finish within its time-slice,the process is lowered to the next lower-priority queue. As some CPU-intensive processes may starve because of this implementation, we perform aging, where, after they exceed the `AGING_TICKS` time (defined in src/kernel/param.h) in their current queue, they are sent to the next higher priority queue.
#### Implementation
A struct named `queue` is introduced in `proc.h`. It consists of an array of proc pointers procList[NPROC]along with a `top`, indicative of the last process enqued. a `capacity` of NPROC (maximum number of processes that can be present at a time), `procCount` (number of processes in the queue currenly) and `sliceTime` for the given queue, as defined above. We create a array of queue f size 4.
To allow for queue management, more attributes are added to proc struct in src/kernel/proc.h. These include `isQueuedFlag` (to track if a process is in any queue), `queueIndex` (the queue in which the process is present), `tickedFor` (to track ticks for which the process has been in the current queue) and `enquedAt` (time of insertion into the current queue). `toUpdate` is a flag that indicates if the process in a queue, on being enqued after a user interrupt needs its values updated (enquedAt set to current `ticks`, `tickedFor` set to 0; this is done for processes sent to a new queue).
In allocproc, we initialize the following for  process:
- p->queueIndex = 0;
- p->isQueuedFlag = 0;
- p->tickedFor = 0;
- p->toUpdate = 0;
In `queue.c`, we have the following functions:
- initQueues: Initializes an array of queues (fbqs), setting their properties and slice times based on predefined constants.

- isFull and isEmpty: Check if a specified queue is full or empty based on the number of processes it contains.

- enque: Adds a process to a specified queue, updating relevant process attributes and timestamps.

- remove: Removes a specific process from its current queue, adjusting the queue's content accordingly.

- deque: Removes and returns the first process from a specified queue.

During processing, in `proc.c`, all processes within the list of processes that are not in any queue and are marked as RUNNABLE are added to their respective queues based on their queueIndex value, utilizing the enque function. 

Additionally, there is a mechanism for selecting the appropriate queue for execution. The system iterates through the queues starting from queue 0 until it finds a non-empty queue with atleast one RUNNABLE process. If any RUNNABLE process goes to `sleep` during its time in the queue, this mechanism deques it. It then allocates CPU time to the first RUNNABLE process found, setting its state to `RUNNING`.

At each clock interrupt, in user trap, the aging function, found in src/kernel/trap.c, is invoked. It moves runnable processes that have been waiting in non-highest-priority queues for `AGING_TICKS` or more time to the higher-priority queue just above, preventing them from being starved and ensuring fair CPU allocation. After this we first check in higher priority queues than the queue of the current process if any RUNNABLE process is present (by aging), in which case we yield and return control so that the scheduler can run that process first by going through the above queues again. If no such process is found, we increment tickedFor by 1, i.e. the process has run for one more tick. Now, we check if the process has used up its time slice (sliceTime). We set toUpdate to 0, only to set it to 1 in case the process is not in the least-priority queue in which we will not decrease its priority. Otherwise we demote the process to the lower-priority queue just after it. If the process has been killed, it exits with an error code (-1) and returns. It then yields the CPU and returns to user mode. If the slicetime has not exceeded however, we do not yield() as we must run the process for sliceTIme ticks. If the process has been killed, it exits with an error code (-1) and returns, otherwise We call usertrapret to return to user mode.
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
In the MLFQ scheduling graph, processes are represented as they move between different priority queues. As processes enter low-priority queues, they rise upwards on the y-axis of the graph, indicating their transition to lower priority. This happens when a process has consumed its entire time slice in the current queue. Conversely, processes descend on the y-axis when they enter higher-priority queues, which occurs after they exceed the AGING_TICKS time in their current queue, as determined by the aging function. This representation captures the scheduling behavior of processes as they compete for CPU time across multiple priority levels in the MLFQ scheduler.


