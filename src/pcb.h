#pragma once
#include <stddef.h>
#include <stdio.h> 
#define PAGE_SIZE 2

typedef size_t pid;

struct PCB {
    pid pid;
    char *name;
    size_t line_base;
    size_t line_count;
    size_t duration;
    size_t pc;
    struct PCB *next;

    size_t line_loaded;  // keep track where in file
    int *page_table[FRAME_STORE_SIZE/FRAME_SIZE]; // page table
};


int pcb_has_next_instruction(struct PCB *pcb);
size_t pcb_next_instruction(struct PCB *pcb);
struct PCB *create_process_already(struct PCB *pcb);
struct PCB *create_process(const char *filename);
struct PCB *create_process_from_FILE(FILE *f);
void free_pcb(struct PCB *pcb);

