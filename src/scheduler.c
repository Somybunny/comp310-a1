#include "scheduler.h"
#include "readyqueue.h"
#include "shellmemory.h"
#include "shell.h"
#include "pcb.h"
#include "interpreter.h"

#include <pthread.h>

#define NUM_WORKERS 2

pthread_t workers[NUM_WORKERS];
int mt_enabled = 0;
int rr_slice = 2; // will store 2 (RR) or 30 (RR30)

pthread_mutex_t rq_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t rq_cond = PTHREAD_COND_INITIALIZER;


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

void *worker_rr(void *arg) {
    while (1) {

        pthread_mutex_lock(&rq_mutex);

        while (rq_is_empty()) {
            if (!mt_enabled) {
                pthread_mutex_unlock(&rq_mutex);
                pthread_exit(NULL);
            }
            pthread_cond_wait(&rq_cond, &rq_mutex);
        }

        PCB *p = dequeue();
        pthread_mutex_unlock(&rq_mutex);

        int count = 0;
        while (count < rr_slice && p->current < p->length) {
            char *instruction = get_program_line(p->start + p->current);
            parseInput(instruction);
            p->current++;
            count++;
        }

        if (p->current < p->length) {
            pthread_mutex_lock(&rq_mutex);
            enqueue(p);
            pthread_cond_signal(&rq_cond);
            pthread_mutex_unlock(&rq_mutex);
        } else {
            destroy_pcb(p);
        }
    }
    return NULL;
}

void scheduler_start_mt(int slice) {
    rr_slice = slice;
    mt_enabled = 1;

    for (int i = 0; i < NUM_WORKERS; i++) {
        pthread_create(&workers[i], NULL, worker_rr, NULL);
    }
}

void scheduler_stop_mt() {
    mt_enabled = 0;

    pthread_mutex_lock(&rq_mutex);
    pthread_cond_broadcast(&rq_cond);
    pthread_mutex_unlock(&rq_mutex);

    for (int i = 0; i < NUM_WORKERS; i++) {
        pthread_join(workers[i], NULL);
    }
}
