#ifndef PCB_H
#define PCB_H

typedef struct PCB {
    int pid;
    int start;
    int length;
    int pc;
    int job_length_score;   // for SJF / AGING later
    struct PCB *next;
} PCB;

PCB* create_pcb(int start, int length);
void destroy_pcb(PCB *pcb);

#endif
