[![Review Assignment Due Date](https://classroom.github.com/assets/deadline-readme-button-24ddc0f5d75046c5622901739e7c5dd533143b0c8e959d652212380cedb1ea36.svg)](https://classroom.github.com/a/DLipn7os)
# Intro to Xv6
OSN Monsoon 2023 mini project 2

## Some pointers
- main xv6 source code is present inside `initial_xv6/src` directory. This is where you will be making all the additions/modifications necessary for the first 3 specifications. 
- work inside the `networks/` directory for the Specification 4 as mentioned in the assignment.
- Instructions to test the xv6 implementations are given in the `initial_xv6/README.md` file. 

- You are free to delete these instructions and add your report before submitting. 

# SCHEDULING
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

# NETWORKS
## Implementing TCP functionality: A Report
### How does my implementation of data sequencing and retransmission differ from traditional TCP? 
This part of the project aims at implementing TCP-like functionality using UDP sockets. However, there are many implementation differences between the two:
### Data Sequencing:
#### UDP-based Implementation
UDP has no built-in sequencing mechanism and has been explicitly handled. In my implementation, data sequencing is achieved through the assignment of sequence numbers to packets by the sender. 
- The nth packet has it's sequence number simply as n-1. 
- The first packet always has the sequence number zero. This is because only one sender is present.
- The sequence number has no information about the number of bytes being transmitted in the current packet or bytes that have already been transmitted prior to this.

This causes us to settle on a predetermined agreed-upon number of bytes to be sent and received by the server and the client in each packet. This value does not change during run-time and thus flow control and congestion control is not performed.
#### Actual TCP
TCP, on the other hand, provides automatic and built-in data sequencing. In the TCP header for each packet, a sequence number is assigned to it which ensures that data is delivered to the receiver in the correct order. This sequence numnber tracks the number of bytes that are being sent and is thus more complicated than our implementation. 
- The first sequence number is randomly chosen by the senders. This is to distinguish between multiple clients and servers sending the data.
- After this, each sequence number is the sum of the previous sequence number with the number of bytes transmitted through the previous packet.
- Thus, the sequence number carries information about the number of bytes transmitted.

It thus allows the sender to modulate the number of bytes transmitted to the sender based on information it receives from its receive window (number of bytes the receiver can accept), thus allowing flow control.

### Retransmission:

##### The key difference between my implementaion and TCP is the retransmission timeout interval in TCP. In TCP RTO is calculated using the following:
- EstimatedRTT = (1- $\alpha$) x EstimatedRTT + $\alpha$ x SampleRTT
- DevRTT = (1-b) x DevRTT + $\beta$ x $\beta$ x | EstimatedRTT-SampleRTT | 
- TimeoutInterval = EstimatedRTT + 4 x DevRTT
##### where,
- SampleRTT: Time measured from segment transmission until ACK receipt
- EstimatedRTT: Estimated weighted moving average (EWMA) α = 0.25
- DevRTT: EWMA of sampleRTT deviation from EstimatedRTT β = 0.75
- TimeoutInterval (RTO): Estimated Time + some kind of safety margin

##### In my implementation the sender does not wait to retransmit the data. Unlike actual TCP, it retransmits a packet only after all packets that are serially after it have been transmitted atleast once. Following this, the retransmission of all unacknowledged packets are attempted for 1000 $\mu$ s at a time in a roundrobin fashion until the ACK for all packets is received.

#### Detailed description of both protocols:
#### UDP-based Implementation
In my implementation, retransmission of lost or unacknowledged packets is handled using the following strategy: The sender maintains an array for each packet indicative of whether the acknowledgement for it has been received or not. Initially, for each communication, it is initialized to 0 for all packets. The sender uses a non-blocking recvfrom() to wait for 1000 microseconds for ACK to be received after sending the packet (say, the i<sub>th</sub> packet). The acknowledgement for a packet is simply the sequence number corresponding to it. If it is received in time, the i<sub>th</sub> index of the acknowledgement array is set to 1.

After the sender sends all packets once, it checks if it has received the acknowledgement for all transmitted packets by maintaining a count of number of unique acknowledgements (unique meaning ACK array entry for that index is not already 1, in orde to not recount a packet). If not, the sender retransmits all such unacknowledged packets (packets with sequence number such that that index in acknowledgement array entry is 0). It does so until the number of unique acknowledgements received is equal to the total number of packets (chunkCount). 

After all packets are received, the receiver arranges the packets in increasing order of sequence numbers by storing it in an array to construct the data sent.
#### Actual TCP
TCP incorporates a sophisticated retransmission mechanism. When a TCP sender transmits a data segment, it starts a timer associated with that segment. This timer is often referred to as the `retransmission timeout`. If the sender does not receive an acknowledgment for the transmitted segment within the RTO period, it assumes that the segment was lost or not delivered and retransmits the unacknowledged segment. The RTO value is dynamic and adapts based on network conditions. It typically starts with a conservative value and increases or decreases as the sender gains more information about the network's behavior.

TCP maintains a record of all sent segments and their acknowledgment status. If some segments are acknowledged while others are not, TCP will selectively retransmit only the missing or unacknowledged segments. 

To reduce overhead, TCP may implement delayed acknowledgments, where the receiver waits for a certain condition before sending acknowledgments. This condition is often either receiving two segments (cumulative acknowledgment) or waiting for a short timeout (N seconds).

### Extending my implementation to account for flow control
TCP uses a `sliding window` flow control protocol. In each TCP segment, the receiver specifies in the receive window field the amount of additionally received data (in bytes) that it is willing to buffer for the connection. The sending host can send only up to that amount of data before it must wait for an acknowledgement and receive window update from the receiving host.

We will try to implement a similar form of flow control in our UDP implementation of TCP:
#### Changes in code:
- The sender `S` no longer divides packets into fixed size chunks, but does it dynamically based on the information sent by the receiver `R`.
- We modify the acknowlegement sent by R from an int value to a struct, containing both the `ACK number` for the given packet and also a `Window` for the next packet.
- We modify the sender's 'data' struct to include an integer for the number of bytes being transmitted in the packet.
#### Changes in implementation logic:
- S and R establish a communication. The initial window size, denoted as W, is agreed upon by S and R. 
- S maintains a sending window S<sub>W</sub> that represents the range of sequence numbers it is allowed to send. Initially, S<sub>W</sub> covers the first W packets, 0 to W-1.
- In the ACK packet sent back to S, R includes an advertised window size R<sub>W</sub>. This value indicates the number of packets R's buffer can currently accommodate.
- When S receives an ACK with R<sub>W</sub>, it adjusts its sending window S<sub>W</sub> accordingly. The new S<sub>W</sub> is set to start from the first byte never transmitted yet, say the i<sup>th</sup> byte to the [i+ min(S<sub>W</sub>, R<sub>W</sub>)-1]<sup>th</sup> byte. The bytes in this sending window are transmitted in the next packet.
- S continues to transmit packets within the adjusted S<sub>W</sub> while considering R<sub>W</sub>. Both S and R continue to adjust their window sizes based on network conditions. 
- If R's buffer fills up and it cannot process packets fast enough, it may reduce R<sub>W</sub> in its ACKs to signal S to slow down. If S detects congestion or packet loss, it may reduce its sending window size to prevent further congestion. R<sub>W</sub> is sent as 0 if the receiver is unable to hold more data temporarily.
- This process continues iteratively, with S and R dynamically adjusting their window sizes based on the receiver's buffer capacity and network conditions.

Thus, a simple form of flow control has been implemented using an `advertised window` specified by the receiver in the ACK packet it sends back, specifying the amount of additionally received data (in bytes) that it is willing to buffer for the connection.



