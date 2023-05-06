#include "../include/simulator.h"
#include <iostream>
#include "../include/simulator.h"
#include <queue>
#include <cstring>
#include <cstdio>


#include "../include/simulator.h"


std::vector<struct msg> packetList;

int winSize;
int baseSeqNum;
int nextSeqNum;
int seqNumB;


/* ******************************************************************
 ALTERNATING BIT AND GO-BACK-N NETWORK EMULATOR: VERSION 1.1  J.F.Kurose

   This code should be used for PA2, unidirectional data transfer 
   protocols (from A to B). Network properties:
   - one way network delay averages five time units (longer if there
     are other messages in the channel for GBN), but can be larger
   - packets can be corrupted (either the header or the data portion)
     or lost, according to user-defined probabilities
   - packets will be delivered in the order in which they were sent
     (although some can be lost).
**********************************************************************/

/********* STUDENTS WRITE THE NEXT SEVEN ROUTINES *********/

int checksum(struct pkt packet){
//    printf("checksum: ack no %d seq no  %d payload %s\n", packet.acknum, packet.seqnum, packet.payload);
    int n = sizeof(packet.payload)/sizeof(packet.payload[0]);
    int sum = 0;
    for(int i =0; i<n; i++){
        sum+= (int)packet.payload[i];
    }
    sum += packet.seqnum + packet.acknum;
    return sum;
}

struct pkt create_packet(){
	pkt next_pkt;
    next_pkt.seqnum= nextSeqNum;
    next_pkt.acknum= 0;
    strcpy(next_pkt.payload, packetList[nextSeqNum].data);
    next_pkt.checksum= checksum(next_pkt);

    
    int max_seq_num = baseSeqNum + winSize;
    while(nextSeqNum < max_seq_num && nextSeqNum < packetList.size()) {
       
        tolayer3(0, next_pkt);
            
        if(baseSeqNum == nextSeqNum){
           
            starttimer(0, 20);
        }
            
        nextSeqNum++;
    }
}

bool check_corruption(struct pkt packet, int check_sum){
//    printf("cheksum: %d, %d, seq: %d, payload: %s\n", packet.checksum, check_sum, packet.seqnum, packet.payload);
//    printf("ack no: %d, %d\n", packet.acknum, check_sum);

    if(packet.checksum == check_sum){
        return true;
    }
    return false;
}

/* called from layer 5, passed the data to be sent to other side */
void A_output(struct msg message)
{
	packetList.push_back(message);
	create_packet();
}

/* called from layer 3, when a packet arrives for layer 4 */
void A_input(struct pkt packet)
{
	int packChecksum= checksum(packet);
    if(check_corruption(packet, packChecksum)){
        baseSeqNum = packet.acknum + 1;
		
		if(baseSeqNum == nextSeqNum)
			stoptimer(0);
		else{
            stoptimer(0);
			starttimer(0, 20);
		}
    }
}

/* called when A's timer goes off */
void A_timerinterrupt()
{
	nextSeqNum= baseSeqNum;
	create_packet()
}  

/* the following routine will be called once (only) before any other */
/* entity A routines are called. You can use it to do any initialization */
void A_init()
{
	winSize= getwinsize();
	baseSeqNum=0;
	nextSeqNum=0;
}

/* Note that with simplex transfer from a-to-B, there is no B_output() */

/* called from layer 3, when a packet arrives for layer 4 at B*/
void B_input(struct pkt packet)
{
	int checkSumB= checksum(packet);
	if(check_corruption(packet, checkSumB) && seqNumB == packet.seqnum){// add packet corrupttion check
		//cout<< "**********Sending data to layer 5 of B********* "<<endl;
		tolayer5(1, packet.payload);
		
		pkt ackB;
		
		ackB.acknum = seqNumB;
		ackB.checksum = 0;
		
		ackB.checksum = checksum(ackB);
		
		tolayer3(1, ackB);	
		
		seqNumB++;	
	}
}

/* the following rouytine will be called once (only) before any other */
/* entity B routines are called. You can use it to do any initialization */
void B_init()
{
	seqNumB= 0;
}
