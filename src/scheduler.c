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
        
	// Run all instructions
        while (p->current < p->length) {
            char *instruction = get_program_line(p->start + p->current);
            errCode = parseInput(instruction);
            p->current++;
        }

        destroy_pcb(p);
    }

    // Clean memory
    reset_program_memory();
    return errCode;
}

int scheduler_run_RR(int nb_instructions) {
    int errCode = 0;

    while (!rq_is_empty()) {

	PCB *p = dequeue();
        int count = 0;

	while (count < nb_instructions && p->current < p->length) {
	    char *instruction = get_program_line(p->start + p->current);
            errCode = parseInput(instruction);
	    p->current++;
	    count++;
	}

	if (p->current < p->length) {
            enqueue(p);
	} else {
	    destroy_pcb(p);
	}
    }

    reset_program_memory();
    return errCode;
}


int scheduler_run_aging(){
    int errCode = 0;
    while (!rq_is_empty()) {
        PCB *p = dequeue();

        // Run one instruction
        if (p->current < p->length) {
            char *instruction = get_program_line(p->start + p->current);
            errCode = parseInput(instruction);
            p->current++;
        }

        age_queue();

        // Re-insert program
        if (p->current < p->length) {
            enqueue_aging(p);
        } else {
            destroy_pcb(p);
        }
    }

    // Clean memory
    reset_program_memory();
    return errCode;
}

