#ifndef _SEARCH_H
#define _SEARCH_H

#include "state.h"
#include "move.h"
#include "plp.h"

#define MAX_DEPTH 256

typedef struct
{
    int moves[MAX_DEPTH];
    int count;
} pv_t;

typedef struct
{
    int best_score;
    int max_depth;

    int move_phase[MAX_DEPTH];

    uint64_t visited_nodes;
    uint64_t qs_visited_nodes;
    int cache_hits, cache_misses;
    int eval_cache_hits, eval_cache_misses;

    pv_t pv;

    int in_endgame;
    int null_depth; /* for each null move, increment, when we come back, decrement */

#ifdef USE_KILLERS
    int killers[MAX_DEPTH][2];
#endif
} search_data_t;

extern search_data_t search;

void search_go(state_t*, int);

void search_iterative(state_t*, int);
int search_ab(state_t*, int, int, int, int, int, pv_t*, int);

#endif
