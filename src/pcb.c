#include <stdio.h>
#include <stdlib.h>
#include <string.h> // memset
#include "shell.h" // MAX_USER_INPUT
#include "shellmemory.h"
#include "pcb.h"
#include "interpreter.h"
#include "queue.h"

static pid fresh_pid = 1;

void handle_page_fault(struct PCB *pcb, int page); 

int pcb_has_next_instruction(struct PCB *pcb) {
    return pcb->pc < pcb->line_count;
}

size_t pcb_next_instruction(struct PCB *pcb) {
    int page = pcb->pc / FRAME_SIZE;
    int offset = pcb->pc % FRAME_SIZE;

    // page fault
    if (pcb->page_table[page] == -1) {
        handle_page_fault(pcb, page);
	return PAGE_FAULT_SIGNAL;
    }

    // set to next inst
    int frame = pcb->page_table[page];

    size_t index = frame * FRAME_SIZE + offset;

    pcb->pc++;

    return index;
}

struct PCB *create_process_already(struct PCB *pcb) {
    // check if pcb exists
    if (!pcb) {
        perror("failed to create new pcb for create_process_already");
	return NULL;
    }

    // copy data
    struct PCB *new_pcb = malloc(sizeof(struct PCB));
    new_pcb->pid = fresh_pid++;
    new_pcb->name = pcb->name;
    new_pcb->next = NULL;
    new_pcb->pc = 0;
    new_pcb->line_count = pcb->line_count;
    new_pcb->line_base = pcb->line_base;
    new_pcb->duration = pcb->duration;
    new_pcb->line_loaded = pcb->line_loaded;
    memcpy(new_pcb->page_table, pcb->page_table, sizeof(pcb->page_table));
    return new_pcb;
}

struct PCB *create_process(const char *filename) {
    FILE *script = fopen(filename, "rt");
    if (!script) {
        perror("failed to open file for create_process");
        return NULL;
    }
    struct PCB *pcb = create_process_from_FILE(script);
    pcb->name = strdup(filename);
    return pcb;
}


struct PCB *create_process_from_FILE(FILE *script) {
    struct PCB *pcb = malloc(sizeof(struct PCB));
    pcb->pid = fresh_pid++;
    pcb->name = "";
    pcb->next = NULL;
    pcb->pc = 0;
    pcb->line_count = 0;
    pcb->line_base = 0;
    pcb->line_loaded = 0;
    char linebuf[MAX_USER_INPUT];
    size_t lines_loaded = 0;

    // initialize page table
    for (int i = 0; i < (100); i++) {
        pcb->page_table[i] = -1;
    }

    // loop for 2 pages or until no lines
    while (!feof(script) && lines_loaded < PAGE_SIZE * FRAME_SIZE) {
        memset(linebuf, 0, sizeof(linebuf));
        fgets(linebuf, MAX_USER_INPUT, script);

        size_t index = allocate_line(linebuf);
        if (index == (size_t)(-1)) {
            free_pcb(pcb);
            fclose(script);
            return NULL;
        }

        if (pcb->line_count == 0) {
            pcb->line_base = index;
        }

        pcb->line_count++;
	int page = lines_loaded++ / FRAME_SIZE;
	pcb->page_table[page] = index / FRAME_SIZE;
    }

    // count remaining lines
    while (fgets(linebuf, MAX_USER_INPUT, script)) {
        pcb->line_count++;
    }

    fclose(script);
    pcb->duration = pcb->line_count;
    pcb->line_loaded = lines_loaded;
    align_to_next_page();

    return pcb;
}

void free_pcb(struct PCB *pcb) {
    //for (size_t ix = pcb->line_base; ix < pcb->line_base + pcb->line_count; ++ix) {
    //    free_line(ix);
    //}
    if (strcmp("", pcb->name)) {
        free(pcb->name);
    }
    free(pcb);
}

// page fault handler
void handle_page_fault(struct PCB *pcb, int page) {
    printf("Page fault!\n");
    // check if free frame
    int frame = find_free_frame();

    // find evict frame
    if (frame == -1) {
        frame = pick_victim_frame();

	// print victim frame content
        print_victim(frame);

	// update all page tables
        evict_frame(frame);
    }

    // load the line in frame
    load_page_into_frame(pcb, page, frame);
    
    // put the frame in page table
    pcb->page_table[page] = frame;
}
