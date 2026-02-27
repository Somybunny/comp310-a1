#include "scheduler.h"
#include "readyqueue.h"
#include "shellmemory.h"
#include "pcb.h"
#include "interpreter.h"

void scheduler_run() {

    while (!rq_is_empty()) {

        PCB *p = dequeue();

        while (p->current < p->length) {
            char *instruction = get_program_line(p->start + p->current);
            interpret(instruction);
            p->current++;
        }

        destroy_pcb(p);
    }

    reset_program_memory();
}

