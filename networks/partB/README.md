## Implementing TCP functionality: A Report
### How does my implementation of data sequencing and retransmission differ from traditional TCP? 
This part of the project aims at implementing TCP-like functionality using UDP sockets. However, there are many implementation differences between the two:
### Data Sequencing:
#### UDP-based Implementation
UDP has no built-in sequencing mechanism and has been explicitly handled. In my implementation, data sequencing is achieved through the assignment of sequence numbers to packets by the sender. The nth packet has it's sequence number simply as n-1, with the first packet having the sequence number zero. The sequence number has no information about the number of bytes being transmitted in the current packet or bytes that have already been transmitted prior to this.

This causes us to settle on a predetermined agreed-upon number of bytes to be sent and received by the server and the client in each packet. This value does not change during run-time and thus flow control and congestion control is not performed.
#### Actual TCP
TCP, on the other hand, provides automatic and built-in data sequencing. In the TCP header for each packet, a sequence number is assigned to it which ensures that data is delivered to the receiver in the correct order. This sequence numnber tracks the number of bytes that are being sent and is thus more complicated than our implementation. Initial sequence numbers are randomly chosen by the senders. After this, each sequence number is the sum of the previous sequence number with the number of bytes transmitted through the previous packet.

It thus allows the sender to modulate the number of bytes transmitted to the sender based on information it receives from its receive window (number of bytes the receiver can accept), thus allowing flow control.

### Retransmission:
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