#ifndef _TIMECTRL_H
#define _TIMECTRL_H

#include <sys/time.h>
#include "state.h"

typedef struct
{
    int searching;
    int no_more_root_nodes;
    int wtime;
    int btime;
    int ponder;
    int depth;
    int nodes;
    int infinite;
    int search_time_left;
    struct timeval start_time;
} timecontrol_t;

extern timecontrol_t timecontrol;
void timectrl_go(state_t*, int, int, int, int, int, int);
void timectrl_alarm(int);

#endif
