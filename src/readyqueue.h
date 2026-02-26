#ifndef READYQUEUE_H
#define READYQUEUE_H

#include "pcb.h"

void rq_init();
void enqueue(PCB *pcb);
PCB* dequeue();
int rq_is_empty();
void rq_clear();

#endif
