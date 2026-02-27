#include <stddef.h>
#include "readyqueue.h"

static PCB *head = NULL;
static PCB *tail = NULL;


void rq_init() {
    head = NULL;
    tail = NULL;
}


void enqueue(PCB *pcb) {
    pcb->next = NULL;

    // If list empty
    if (tail == NULL) {
        head = pcb;
        tail = pcb;
    } 
    else {
        tail->next = pcb;
        tail = pcb;
    }
}


PCB* dequeue() {
    // Check if empty
    if (head == NULL) { 
        return NULL;
    }

    PCB *temp = head;
    head = head->next;

    // Check if empty after dequeue
    if (head == NULL) {
        tail = NULL;
    }

    return temp;
}


int rq_is_empty() {
    return head == NULL;
}


void rq_clear() {
    PCB *curr = head;
    while (curr != NULL) {
        PCB *next = curr->next;
     	destroy_pcb(curr);
        curr = next;
    }

    head = NULL;
    tail = NULL;
}


void enqueue_length(PCB *pcb) {
    pcb->next = NULL;

    // Check if empty
    if (head == NULL) {
        head = tail = pcb;
        return;
    }

    // Sort placement
    PCB *curr = head;
    PCB *prev = NULL;
    while (curr && curr->job_length_score < pcb->job_length_score) {
        prev = curr;
        curr = curr->next;
    }

    // Update placement
    pcb->next = curr;
    if (prev != NULL) {
        prev->next = pcb;
    }
    else {
        head = pcb;
    }

    if (curr == NULL) {
	    tail = pcb;
    }
}

// Helper for aging 
void age_queue() {
    PCB *curr = head;
    while (curr) {
        if (curr->job_length_score > 0) {
            curr->job_length_score--;
        }
        curr = curr->next;
    }
}

