#include "readyqueue.h"

static PCB *head = NULL;
static PCB *tail = NULL;

void rq_init() {
    head = NULL;
    tail = NULL;
}

void enqueue(PCB *pcb) {
    pcb->next = NULL;

    if (tail == NULL) {
        head = pcb;
        tail = pcb;
    } else {
        tail->next = pcb;
        tail = pcb;
    }
}

PCB* dequeue() {
    if (head == NULL) return NULL;

    PCB *temp = head;
    head = head->next;

    if (head == NULL)
        tail = NULL;

    return temp;
}

int rq_is_empty() {
    return head == NULL;
}

void rq_clear() {
    PCB *curr = head;
    while (curr != NULL) {
        PCB *next = curr->next;
     	destroy_pcd(curr);
        curr = next;
    }

    head = NULL;
    tail = NULL;
}
