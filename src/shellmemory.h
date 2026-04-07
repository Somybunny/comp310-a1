#include <stdio.h>
#define MEM_SIZE 1000
#define FRAME_SIZE 3

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
