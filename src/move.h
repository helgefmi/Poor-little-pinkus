#ifndef _MOVE_H
#define _MOVE_H

#include "state.h"

/* Contains all the information of a move on the chess board */
typedef struct
{
    uint64_t from_square, to_square;
    int from_square_idx, to_square_idx;
    int from_piece, capture;
    int promotion;

    /* Used for unmake_move */
    uint64_t castling, en_passant;
} move_t;

void move_to_string(move_t*, char*);
void move_generate_moves(state_t*, move_t*, int*);
uint64_t move_piece_moves(state_t*, int, int, int);
int move_is_attacked(state_t*, uint64_t, int);

void move_make(state_t*, move_t*);
void move_unmake(state_t*, move_t*);

#endif
