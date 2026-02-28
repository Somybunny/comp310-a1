#include "scheduler.h"
#include "readyqueue.h"
#include "shellmemory.h"
#include "shell.h"
#include "pcb.h"
#include "interpreter.h"

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define NUM_WORKERS 2

int scheduler_running = 0;

pthread_t workers[NUM_WORKERS];
int mt_enabled = 0;
int rr_slice = 2; // will store 2 (RR) or 30 (RR30)
int active_workers = 0;
int quit_called = 0;

pthread_mutex_t rq_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t rq_cond = PTHREAD_COND_INITIALIZER;


int scheduler_is_running(){
	return scheduler_running;
}

int scheduler_is_worker_thread() {
    if (!mt_enabled) return 0;
    pthread_t self = pthread_self();
    for (int i = 0; i < NUM_WORKERS; i++) {
        if (pthread_equal(self, workers[i])) return 1;
    }
    return 0;
}


int scheduler_run() {
    scheduler_running = 1;
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
    scheduler_running = 0;
    return errCode;
}

int scheduler_run_RR(int nb_instructions) {
    scheduler_running = 1;
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
    scheduler_running = 0;
    return errCode;
}


int scheduler_run_aging(){
    scheduler_running = 1;
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
            enqueue_length(p);
        } else {
            destroy_pcb(p);
        }
    }

    // Clean memory
    reset_program_memory();
    scheduler_running = 0;
    return errCode;
}

void *worker_rr(void *arg) {
    while (1) {

        pthread_mutex_lock(&rq_mutex);

        while (rq_is_empty()) {
            if (!mt_enabled && active_workers == 0) {
		    pthread_cond_broadcast(&rq_cond);
                pthread_mutex_unlock(&rq_mutex);
                pthread_exit(NULL);
            }
            pthread_cond_wait(&rq_cond, &rq_mutex);
        }

        PCB *p = dequeue();
	active_workers++;
        pthread_mutex_unlock(&rq_mutex);

        int count = 0;
        while (count < rr_slice && p->current < p->length) {
            char *instruction = get_program_line(p->start + p->current);
            parseInput(instruction);
            p->current++;
            count++;
        }

	pthread_mutex_lock(&rq_mutex);
        active_workers--;
        if (p->current < p->length) {
            enqueue(p);
        } else {
            destroy_pcb(p);
        }
	pthread_cond_broadcast(&rq_cond);
        pthread_mutex_unlock(&rq_mutex);
    }
    return NULL;
}

void scheduler_start_mt(int slice) {
    if (mt_enabled) {
        // Recursive exec while MT already running:
        // PCBs already enqueued by exec(), just wake workers
        pthread_mutex_lock(&rq_mutex);
        pthread_cond_broadcast(&rq_cond);
        pthread_mutex_unlock(&rq_mutex);
        return;
    }

    rr_slice = slice;
    mt_enabled = 1;
    active_workers = 0;
    quit_called = 0;

    for (int i = 0; i < NUM_WORKERS; i++) {
        pthread_create(&workers[i], NULL, worker_rr, NULL);
    }

    pthread_mutex_lock(&rq_mutex);
    while ((rq_is_empty() == 0 || active_workers > 0) && !quit_called) {
        pthread_cond_broadcast(&rq_cond);
        pthread_cond_wait(&rq_cond, &rq_mutex);
    }
    mt_enabled = 0;
    pthread_cond_broadcast(&rq_cond);
    pthread_mutex_unlock(&rq_mutex);

    for (int i = 0; i < NUM_WORKERS; i++) {
        pthread_join(workers[i], NULL);
    }

    reset_program_memory();
    if (quit_called) {
        exit(0);
    }
}

int scheduler_is_mt_running() {
    return mt_enabled;
}

// Called when quit runs from inside a worker thread (batch script with #)
void scheduler_worker_quit() {
    // Signal all workers to stop after current instruction
    pthread_mutex_lock(&rq_mutex);
    quit_called = 1;
    pthread_cond_broadcast(&rq_cond);
    pthread_mutex_unlock(&rq_mutex);

    // Exit this worker - main thread in scheduler_start_mt will
    // join all workers and then we print Bye! and exit
    pthread_exit(NULL);
}
