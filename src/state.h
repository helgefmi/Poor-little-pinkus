#ifndef _STATE_H
#define _STATE_H

#include <stdint.h>

/* Represents the state of a position on the chess board.
 *
 * This class has the variables:
 * pieces      - Set of bitboards representing the pieces position on the board.
 * turn        - Who's turn it is.
 * castling    - Castling availability.
 * en_passant  - En passant availability.
 * occupied    - Which squares are occupied by white, black, or both. */

typedef struct
{
    uint64_t pieces[2][6];
    int turn;
    uint64_t castling, en_passant;
    uint64_t occupied[2];
    uint64_t occupied_both;
} state_t;

struct move_t;

void state_init_from_fen(state_t*, char*);
void state_print(state_t*);

#endif
