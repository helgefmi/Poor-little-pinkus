#ifndef _SEARCH_H
#define _SEARCH_H

#include "state.h"
#include "move.h"

#define AB_INVALID_NODE -0xdead

typedef struct
{
    int pv[128][128];
    int best_score;
    int max_depth;

    int move_phase[128];

    uint64_t visited_nodes;
    uint64_t qs_visited_nodes;
    int cache_hits, cache_misses;
} search_data_t;

extern search_data_t search;

void search_go(state_t*, int);

void search_iterative(state_t*, int);
int search_ab(state_t*, int, int, int);

#endif
