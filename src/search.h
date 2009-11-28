#ifndef _SEARCH_H
#define _SEARCH_H

#include "state.h"
#include "move.h"

#define AB_INVALID_NODE -0xdead

typedef struct
{
    int move;
    int score;
    int depth;
} pv_t;

typedef struct
{
    pv_t pv[100];
    uint64_t visited_nodes;
    int cache_hits, cache_misses;
    int max_depth;
} search_data_t;

extern search_data_t search_data;

void search_go(state_t*, int);

void search_iterative(state_t*, int);
int search_ab(state_t*, int, int, int);

#endif
