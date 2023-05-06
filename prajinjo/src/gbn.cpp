#include "../include/simulator.h"
#include <cstdlib> // for malloc and free
#include <cstring> // for memcpy

#define NULL 0
#define A 0
#define B 1
#define TIMEOUT 30.0
#define DEFAULT_ACK 111

/* All function declarations */
int calc_checksum(struct pkt *p);
void append_msg(struct msg *m);
struct node *pop_msg();

/* Global variables */
int B_seqnum = 0;          // Sequence count for B
int nextseq = 0;          // Sequence count for A
int pkt_in_window = 0;    // Number of packets in A's window
int WINDOW = 0;           // Window size (initialized in A_init)
struct pkt *packets;      // Pointer to an array of packets (equal to window size)
struct node {
  struct msg message;
  struct node *next;
};
struct node *list_head = NULL;
struct node *list_end = NULL;
int window_start = 0;     // Packet for which we are waiting for ACK
int last = 0;             // Last transmitted packet from the window
int waiting_ack = 0;      // ACK A's waiting for

/* Called from layer 5, passed the data to be sent to the other side */
void A_output(struct msg message) {
  printf("\n================================ Inside A_output===================================\n");
  struct node *n;
  append_msg(&message);
  if (pkt_in_window == WINDOW) {
    return;
  }
  n = pop_msg();
  if (n == NULL) {
    printf("No message needs to be processed\n");
    return;
  }
  if (((last + 1) % WINDOW) == window_start) {
    return;
  } else {
    if (pkt_in_window != 0) {
      last = (last + 1) % WINDOW;
    }
  }
  packets[last];  // This is the packet we selected
  for (int i = 0; i < 20; i++) {
    packets[last].payload[i] = n->message.data[i];
  }
  free(n); // Free the memory of n
  packets[last].seqnum = nextseq;
  packets[last].acknum = DEFAULT_ACK;
  packets[last].checksum = calc_checksum(&packets[last]);
  nextseq++;
  pkt_in_window++; // Update the number of packets in the window
  tolayer3(A, packets[last]);
  if (window_start == last) {
    starttimer(A, TIMEOUT);
  }
}

/* Called from layer 3, when a packet arrives for layer 4 */
void A_input(struct pkt packet) {
  printf("\n================================ Inside A_input===================================\n");

  if (packet.checksum != calc_checksum(&packet)) {
    printf("Wrong checksum\n");
    return;
  }
  if (packet.acknum != packets[window_start].seqnum) {
    printf("Expected ACK: %d\nWrong ACK: %d\n", packets[window_start].seqnum, packet.acknum);
    return;
  }
  packets[window_start].seqnum = -1; // Set the sequence number of that packet to -1
  stoptimer(A);
  pkt_in_window--; // Decrement the number of packets in the window

  if (pkt_in_window == 0) {
    struct node *n;
    n = pop_msg();
    while (n != NULL) {
      packets[last];
      for (int i = 0; i < 20; i++) {
        packets[last].payload[i] = n->message.data
      }
      free(n); // Free the memory of n
      packets[last].seqnum = nextseq;
      packets[last].acknum = DEFAULT_ACK;
      packets[last].checksum = calc_checksum(&packets[last]);
      nextseq++;
      pkt_in_window++; // Update the number of packets in the window
      tolayer3(A, packets[last]);
      starttimer(A, TIMEOUT);
    }
  } else {
    window_start = (window_start + 1) % WINDOW;
    struct node *n;
    n = pop_msg();
    if (n != NULL) {
      last = (last + 1) % WINDOW;
      packets[last];
      for (int i = 0; i < 20; i++) {
        packets[last].payload[i] = n->message.data[i];
      }
      free(n); // Free the memory of n
      packets[last].seqnum = nextseq;
      packets[last].acknum = DEFAULT_ACK;
      packets[last].checksum = calc_checksum(&packets[last]);
      nextseq++;
      pkt_in_window++; // Update the number of packets in the window
      tolayer3(A, packets[last]);
    }
  }

  if (window_start != last || pkt_in_window == 1) {
    starttimer(A, TIMEOUT);
  }
}

/* Called when A's timer goes off */
void A_timerinterrupt() {
  printf("\n================================ Inside A_timerinterrupt===================================\n");

  int i = window_start;
  printf("Expecting ACK: %d\n", packets[window_start].seqnum);
  while (i != last) {
    printf("Sending seq no: %d\n", packets[i].seqnum);
    tolayer3(A, packets[i]);
    i = (i + 1) % WINDOW;
  }
  printf("Sending seq no: %d\n", packets[i].seqnum);
  tolayer3(A, packets[i]);

  /* If there are still some packets, start the timer again */
  if (window_start != last || pkt_in_window == 1) {
    starttimer(A, TIMEOUT);
  }
}

/* The following routine will be called once (only) before any other */
/* entity A routines are called. You can use it to do any initialization */
void A_init() {
  WINDOW = getwinsize(); // Initialize the WINDOW variable
  packets = (struct pkt *)malloc(sizeof(struct pkt) * WINDOW); // Allocate memory for packets in the window
}

/* Note that with simplex transfer from A to B, there is no B_output() */

/* Called from layer 3, when a packet arrives for layer 4 at B */
void B_input(struct pkt packet) {
  printf("\n================================ Inside B_input===================================\n");

  if (packet.checksum != calc_checksum(&packet)) {
    printf("Packet is corrupted");
    return;
  }
  printf("Expected seq: %d\n", B_seqnum);
  if (packet.seqnum == B_seqnum) {
    printf("Correct packet received, sending it to layer 5");
    ++B_seqnum;
    tolayer5(B, packet.payload);
  } else {
    printf("Packet seq: %d\n", packet.seqnum);
    printf("Out-of-order packet");
    if (packet.seqnum < B_seqnum) {
      printf("Sent ACK: %d\n", packet.seqnum);
      packet.acknum = packet.seqnum;
      packet.checksum = calc_checksum(&packet);
      tolayer3(B, packet);
    }
  }
}

/* The following routine will be called once (only) before any other */
/* entity B routines are called. You can use it to do any initialization */
void B_init() {
  B_seqnum = 0; // Initialize B_seqnum
}

//////////////////////////////////////////////////////////////////////////////
int calc_checksum(struct pkt *p) {
  int i;
  int checksum = 0;

  if (p == NULL) {
    return checksum;
  }
  for (i = 0; i < 20; i++) {
    checksum += (unsigned char)p->payload[i];
  }
  checksum += p->seqnum;
  checksum += p->acknum;
  return checksum;
}

void append_msg(struct msg *m) {
  int i;
  /* Allocate memory */
  struct node *n = (struct node *)malloc(sizeof(struct node));
  if (n == NULL) {
    printf("Not enough memory\n");
    return;
  }
  n->next = NULL;
  /* Copy packet */
  for (i = 0; i < 20; ++i) {
    n->message.data[i] = m->data[i];
  }

  /* If list is empty, just add into the list */
  if (list_end == NULL) {
    list_head = n;
    list_end = n;
    return;
  }
  /* Otherwise, add at the end */
  list_end->next = n;
  list_end = n;
}

struct node *pop_msg() {
  struct node *p;
  /* If the list is empty, return NULL */
  if (list_head == NULL) {
    return NULL;
  }

  /* Retrieve the first node */
  p = list_head;
  list_head = p->next;
  if (list_head == NULL) {
    list_end = NULL;
  }
  return p;
}
