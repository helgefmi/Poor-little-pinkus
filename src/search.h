#ifndef _SEARCH_H
#define _SEARCH_H

#include "state.h"
#include "move.h"

#define AB_INVALID_NODE -0xdead
#define MAX_DEPTH 256

typedef struct
{
    int pv[MAX_DEPTH][MAX_DEPTH];
    int best_score;
    int max_depth;

    int move_phase[MAX_DEPTH];

    uint64_t visited_nodes;
    uint64_t qs_visited_nodes;
    int cache_hits, cache_misses;

    int in_endgame;
    int can_nullmove[MAX_DEPTH];

    int killers[MAX_DEPTH][2];
} search_data_t;

extern search_data_t search;

void search_go(state_t*, int);

void search_iterative(state_t*, int);
int search_ab(state_t*, int, int, int, int);

#endif
