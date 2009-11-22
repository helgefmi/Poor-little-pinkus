#ifndef _SEARCH_H
#define _SEARCH_H

#include "state.h"
#include "move.h"

typedef struct
{
    move_t move;
    int score;
    int depth;
} search_best_move_t;

search_best_move_t *search_best_move;

void search_go(state_t*, int);
int search_ab(state_t*, int, int, int, int);

#endif
