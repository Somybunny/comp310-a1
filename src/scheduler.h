#ifndef SCHEDULER_H
#define SCHEDULER_H

int scheduler_is_running();
int scheduler_run();
int scheduler_run_RR(int nb_instructions);
int scheduler_run_aging();

#endif
