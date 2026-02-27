#ifndef PCB_H
#define PCB_H

typedef struct PCB {
    int pid;
    int start;
    int length;
    int current;
    int job_length_score;   
    struct PCB *next;
} PCB;

PCB* create_pcb(int start, int length);
void destroy_pcb(PCB *pcb);

#endif
