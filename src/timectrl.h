#ifndef _TIMECTRL_H
#define _TIMECTRL_H

#include <stdint.h>
#include <sys/time.h>
#include "state.h"

#define INPUT_INTERVAL 1000000

typedef struct
{
    uint64_t nodes;
    int searching;
    int no_more_root_nodes;
    int wtime;
    int btime;
    int ponder;
    int depth;
    int infinite;
    int search_time_left;
    int verbose;
    int input_timer;
    struct timeval start_time;
    state_t *state;
} timecontrol_t;

extern timecontrol_t timecontrol;
void timectrl_go(state_t*, int, int, int, int, uint64_t, int, int);
void timectrl_alarm(int);

int timectrl_should_halt();
void timectrl_notify_uci();

#endif
