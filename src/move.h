#ifndef _MOVE_H
#define _MOVE_H

#include "state.h"

#define MSCORE_DEFAULT 0
#define MSCORE_CAPTURE 10
#define MSCORE_PROMOTION 100

/* Contains all the information of a move on the chess board */
typedef struct
{
    int from, to;
    int piece, capture;
    int promotion;
} move_t;

void move_to_string(move_t*, char*);
void move_generate_moves(state_t*, move_t*, int*);
uint64_t move_piece_moves(state_t*, int, int, int);
int move_is_attacked(state_t*, int, int);

void move_make(state_t*, move_t*, int);
void move_unmake(state_t*, move_t*, int);

#endif
