#include "scheduler.h"
#include "readyqueue.h"
#include "shellmemory.h"
#include "shell.h"
#include "pcb.h"
#include "interpreter.h"

int scheduler_run() {
    int errCode = 0;
    while (!rq_is_empty()) {

        PCB *p = dequeue();

        while (p->current < p->length) {
            char *instruction = get_program_line(p->start + p->current);
            errCode = parseInput(instruction);
            p->current++;
        }

        destroy_pcb(p);
    }

    reset_program_memory();
    return errCode;
}
