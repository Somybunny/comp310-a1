#include <stdio.h>
#define MEM_SIZE 1000
#define FRAME_SIZE 3

void init_frame_log();
void update_LRU_clock();
int pick_victim_frame();
void touch_frame(int idx);

void var_init();
char *var_get_value(char *var);
void var_set_value(char *var, char *value);


void assert_frame_is_empty(void);
size_t allocate_line(const char *line);
void free_line(size_t index);
const char *get_line(size_t index);
void reset_frame_allocator(void);

int get_frame(size_t line_index);
int get_offset(size_t line_index);
int total_frames(size_t total_lines);
void align_to_next_page();
int find_free_frame();
void print_victime(int frame);
void load_page_into_frame(struct PCB *pcb, int page, int frame);
void evict_frame(int frame);
