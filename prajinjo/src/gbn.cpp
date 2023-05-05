#include "../include/simulator.h"
#include <iostream>
#include "../include/simulator.h"
#include <queue>
#include <cstring>
#include <cstdio>   


std::vector<struct pkt> packetList;
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

/* called from layer 5, passed the data to be sent to other side */
/* the following rouytine will be called once (only) before any other */
/* entity B routines are called. You can use it to do any initialization */
void B_init()
{
    seqNumB = 0;
}

bool isValidPackAtB(pkt pack){
    if(pack.seqnum == seqNumB){
        std::cout << "My number is: 1";
        return true;
    }
    return false;
}

bool validateChecksum(pkt packet, int checksum){
    std::cout << "My number is: 2";
    if(packet.checksum == checksum){
        std::cout << "My number is: 3";
        return true;
    }
    return false;
    
}


int calcChecksum(int seq, int ack, char *message){
    std::cout << "My number is: 4";
    int n = sizeof(message)/sizeof(message[0]);
    std::cout << "My number is: 5";
    int sum = 0;
    std::cout << "My number is: 6";
    for(int i =0; i<n; i++){
        std::cout << "My number is: 7";
        sum+= (int)message[i];
        std::cout << "My number is: 8";
    }
    sum += seq + ack;
    std::cout << "My number is: 9";
    return sum;
}
void A_output(struct msg message)
{
    std::cout << "My number is: 10";
    pkt next_pkt;
    std::cout << "My number is: 11";
    next_pkt.seqnum= nextSeqNum;
    std::cout << "My number is: 12";
    next_pkt.acknum= -1;
    std::cout << "My number is: 13";
    strcpy(next_pkt.payload, message.data);
    std::cout << "My number is: 14";
    next_pkt.checksum= calcChecksum(next_pkt.seqnum,next_pkt.acknum, next_pkt.payload);
    std::cout << "My number is: 15";
    packetList.push_back(next_pkt);
    std::cout << "My number is: 16";
    int max_seq_num = std::min(baseSeqNum + winSize, static_cast<int>(packetList.size()));
    std::cout << "My number is: 17";
    while(nextSeqNum < max_seq_num) {
    std::cout << "My number is: 18";

        tolayer3(0, next_pkt);
		std::cout << "My number is: 19";
		if(baseSeqNum == nextSeqNum){
            std::cout << "My number is: 20";
			//cout<<"starting timeout in send data for : "<<next_seq_num<<endl;
			starttimer(0, 20);
            std::cout << "My number is: 21";
		}
		std::cout << "My number is: 22";
		nextSeqNum++;
    }
}

/* called from layer 3, when a packet arrives for layer 4 */
void A_input(pkt packet)
{
    std::cout << "My number is: 23";
    int packChecksum= calcChecksum(packet.seqnum, packet.acknum, packet.payload);
    std::cout << "My number is: 24";
    if(validateChecksum(packet, packChecksum) == true){
        std::cout << "My number is: 25";
        baseSeqNum = packet.acknum;
		std::cout << "My number is: 26";
		if(baseSeqNum == nextSeqNum)
        {
            std::cout << "My number is: 27";
			stoptimer(0);
        }
		else{
            std::cout << "My number is: 28";
			stoptimer(0);
            std::cout << "My number is: 29";
			starttimer(0, 20);
		}
    }
    
}

/* called when A's timer goes off */
void A_timerinterrupt()
{
    std::cout << "My number is: 30";
    nextSeqNum = baseSeqNum;
    std::cout << "My number is: 31";
    pkt packA= packetList[nextSeqNum];
    std::cout << "My number is: 32";
    tolayer3(0,packA);
    std::cout << "My number is: 33";
}  

/* the following routine will be called once (only) before any other */
/* entity A routines are called. You can use it to do any initialization */
void A_init()
{
    std::cout << "My number is: 34";
    winSize= getwinsize();
    baseSeqNum=1;
    std::cout << "My number is: 35";
    nextSeqNum=1;
}

/* Note that with simplex transfer from a-to-B, there is no B_output() */

/* called from layer 3, when a packet arrives for layer 4 at B*/
void B_input(struct pkt packet)
{
    std::cout << "My number is: 36";
    bool isPktValid= validateChecksum(packet, calcChecksum(packet.seqnum, packet.acknum, packet.payload));
    bool expValueAtB= isValidPackAtB(packet);
    std::cout << "My number is: 37";
    if(isPktValid && expValueAtB){
        tolayer5(1, packet.payload);
        pkt pktAck;
        std::cout << "My number is: 37";
        pktAck.seqnum= seqNumB;
        pktAck.acknum= 0;
        char ack[] = "Ack Packet"; 
        std::cout << "My number is: 38";
        strcpy(pktAck.payload, ack);
        std::cout << "My number is: 39";
        pktAck.checksum= calcChecksum(pktAck.seqnum,pktAck.acknum, pktAck.payload);
        tolayer3(1, pktAck);
        std::cout << "My number is: 40";
        seqNumB++;
    }
    
}




