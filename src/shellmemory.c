#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <stdio.h>
#include <assert.h>
#include "shellmemory.h"
#include "pcb.h"
#include "shell.h"
#include "interpreter.h"

#define true 1
#define false 0

// LRU bookeeping
int LRU_clock = 0;
int frame_LRU_log[FRAME_STORE_SIZE / FRAME_SIZE];

// Initialize frame_LRU_log to INT_MAX
void init_frame_log() {
    for (int i = 0; i < FRAME_STORE_SIZE / FRAME_SIZE; i++) {
        frame_LRU_log[i] = INT_MAX;
    }
}

void update_LRU_clock() {
    LRU_clock++;
}

int pick_victim_frame() {
    int index = 0;
    int least = INT_MAX;
    for (int i = 0; i < FRAME_STORE_SIZE / FRAME_SIZE; i++) {
        if (frame_LRU_log[i] < least) {
	    least = frame_LRU_log[i];
	    index = i;
	}	
    } 
    return index;
}

void touch_frame(int idx) {
    frame_LRU_log[idx] = LRU_clock;
}


// Helper functions
int match(char *model, char *var) {
    int i, len = strlen(var), matchCount = 0;
    for (i = 0; i < len; i++) {
        if (model[i] == var[i])
            matchCount++;
    }
    if (matchCount == len) {
        return 1;
    } else
        return 0;
}



// for exec memory

struct program_line {
    int allocated; // for sanity-checking
    char *line;
};

struct program_line frame_store[FRAME_STORE_SIZE];
size_t next_free_line = 0;

void reset_frame_allocator() {
    next_free_line = 0;
    assert_frame_is_empty();
}

void assert_frame_is_empty() {
    for (size_t i = 0; i < FRAME_STORE_SIZE; ++i) {
        assert(!frame_store[i].allocated);
        assert(frame_store[i].line == NULL);
    }
}

void init_frame() {
    for (size_t i = 0; i < FRAME_STORE_SIZE; ++i) {
        frame_store[i].allocated = false;
        frame_store[i].line = NULL;
    }
}

size_t allocate_line(const char *line) {
    if (next_free_line >= FRAME_STORE_SIZE) {
        // out of memory!
        return (size_t)(-1);
    }
    size_t index = next_free_line++;
    assert(!frame_store[index].allocated);
    
    update_LRU_clock();
    touch_frame(index / FRAME_SIZE);
    
    frame_store[index].allocated = true;
    frame_store[index].line = strdup(line);
    return index;
}

void free_line(size_t index) {
    update_LRU_clock();
    touch_frame(index / FRAME_SIZE);
    free(frame_store[index].line);
    frame_store[index].allocated = false;
    frame_store[index].line = NULL;
}

const char *get_line(size_t index) {
    update_LRU_clock();
    touch_frame(index / FRAME_SIZE);
    assert(frame_store[index].allocated);
    return frame_store[index].line;
}


// Shell memory functions

struct memory_struct { // block or line
    char *var;
    char *value;
};

struct memory_struct var_store[VAR_STORE_SIZE];



void var_init() {
    int i;
    for (i = 0; i < VAR_STORE_SIZE; i++) {
        var_store[i].var = "none";
        var_store[i].value = "none";
    }
}

// Set key value pair
void var_set_value(char *var_in, char *value_in) {
    int i;

    for (i = 0; i < VAR_STORE_SIZE; i++) {
        if (strcmp(var_store[i].var, var_in) == 0) {
            var_store[i].value = strdup(value_in);
            return;
        }
    }

    //Value does not exist, need to find a free spot.
    for (i = 0; i < VAR_STORE_SIZE; i++) {
        if (strcmp(var_store[i].var, "none") == 0) {
            var_store[i].var = strdup(var_in);
            var_store[i].value = strdup(value_in);
            return;
        }
    }

    return;
}

//get value based on input key
char *var_get_value(char *var_in) {
    int i;

    for (i = 0; i < VAR_STORE_SIZE; i++) {
        if (strcmp(var_store[i].var, var_in) == 0) {
            return strdup(var_store[i].value);
        }
    }
    return NULL;
}

// Frame helpers
// Returns the frame number for a given line index
int get_frame(size_t line_index) {
    return line_index / FRAME_SIZE;
}

// Returns the offset within the frame
int get_offset(size_t line_index) {
    return line_index % FRAME_SIZE;
}

// Returns the total number of frames currently allocated
int total_frames(size_t total_lines) {
    return (total_lines + FRAME_SIZE - 1) / FRAME_SIZE; // ceil division
}

// Put next free lines at the next page 
void align_to_next_page() {
    if (next_free_line % FRAME_SIZE != 0) {
        next_free_line += FRAME_SIZE - (next_free_line % FRAME_SIZE);
    }
}

int find_free_frame() {
    int num_frames = FRAME_STORE_SIZE / FRAME_SIZE;

    for (int f = 0; f < num_frames; f++) {
        int start = f * FRAME_SIZE;

        if (!frame_store[start].allocated &&
            !frame_store[start + 1].allocated &&
            !frame_store[start + 2].allocated) {

            return f;
        }
    }

    return -1;
}

// page fault helper
void load_page_into_frame(struct PCB *pcb, int page, int frame) {
    FILE *f = fopen(pcb->name, "rt");

    char linebuf[MAX_USER_INPUT];

    int start = pcb->line_loaded;

    // skip lines
    for (int i = 0; i < start; i++) {
        fgets(linebuf, MAX_USER_INPUT, f);
    }

    // load 3 lines
    for (int i = 0; i < FRAME_SIZE; i++) {
        if (fgets(linebuf, MAX_USER_INPUT, f)) {
            int idx = frame * FRAME_SIZE + i;

            // Update LRU clock & log
	    update_LRU_clock();
	    touch_frame(frame);

            frame_store[idx].line = strdup(linebuf);
            frame_store[idx].allocated = 1;
        }
    }

    fclose(f);
}


void evict_frame(int frame) {
    for (int i = 0; i < FRAME_SIZE; i++) {
        int idx = frame * FRAME_SIZE + i;

	update_LRU_clock();
	touch_frame(frame);
        free(frame_store[idx].line);
        frame_store[idx].line = NULL;
        frame_store[idx].allocated = 0;
    }

    // CRITICAL: update ALL PCBs
    struct PCB *curr_pcb = get_queue_head();
    while (curr_pcb != NULL) {
	for (int i = 0; i < FRAME_STORE_SIZE / FRAME_SIZE; i++) {
            if (curr_pcb->page_table[i] == frame) {
                curr_pcb->page_table[i] = -1;
	    }
	}
	curr_pcb = curr_pcb->next;
    }
}

void print_victim(int frame) {
    printf("Victim page contents:\n");

    for (int i = 0; i < FRAME_SIZE; i++) {
        int idx = frame * FRAME_SIZE + i;

        if (frame_store[idx].line)
            printf("%s", frame_store[idx].line);
    }

    printf("End of victim page contents.\n");
}


