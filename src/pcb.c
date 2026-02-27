#include <stdlib.h>
#include "pcb.h"

static int next_pid = 1;


PCB* create_pcb(int start, int length) {
    PCB *pcb = malloc(sizeof(PCB));
    pcb->pid = next_pid++;
    pcb->start = start;
    pcb->length = length;
    pcb->current = 0;
    pcb->job_length_score = length;
    pcb->next = NULL;
    return pcb;
}


void destroy_pcb(PCB *pcb) {
    free(pcb);
}
