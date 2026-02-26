#include "scheduler.h"
#include "readyqueue.h"
#include "shellmemory.h"
#include "pcb.h"
#include "interpreter.h"

void scheduler_run() {

    while (!rq_is_empty()) {

        PCB *current = dequeue();

        while (current->pc < current->length) {

            char *instruction =
                get_program_line(current->start + current->pc);

            interpret(instruction);

            current->pc++;
        }

        destroy_pcb(current);
    }

    reset_program_memory();
}
