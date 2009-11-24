#ifndef _SEARCH_H
#define _SEARCH_H

#include "state.h"
#include "move.h"

typedef struct
{
    move_t move;
    int score;
    int depth;
} pv_t;

typedef struct
{
    pv_t pv[100];
    uint64_t visited_nodes;
} search_data_t;

extern search_data_t search_data;

void search_go(state_t*, int);

int search_iterative(state_t*, int);
int search_ab(state_t*, int, int, int, int, int);

#endif