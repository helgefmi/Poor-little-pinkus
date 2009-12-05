#ifndef _STATE_H
#define _STATE_H

#include <stdint.h>

typedef struct
{
    uint64_t pieces[2][6];
    int turn;
    uint64_t castling, en_passant;
    uint64_t occupied[2];
    uint64_t occupied_both;

    uint64_t zobrist;

    int king_idx[2], square[64];

    int last_pawn_or_cap;
    uint64_t repetition[256];
    int moves;

    uint64_t old_castling[256], old_en_passant[256], old_zobrist[256];
    uint64_t old_pawn_or_cap[256];
} state_t;

int state_init_from_fen(state_t*, char*);
void state_print(state_t*);

int state_is_repeating(state_t*);

#endif
