#ifndef _HASHING_H
#define _HASHING_H

#define HASH_EXACT 1
#define HASH_ALPHA 2
#define HASH_BETA 4

#define HASH_EVAL_SIZE (1024 * 1024 * 12)
#define HASH_EVAL_MASK (HASH_EVAL_SIZE - 1)

#include <stdint.h>
#include "state.h"

typedef struct {
    uint64_t pieces[2][6][64];
    uint64_t turn, en_passant[64], castling[64];

    /* The rook mask when castling, given a king color, and move (from_square, to_square). */
    uint64_t rook_castling[2][64][64];

    /* The combined castling hashes for the 3 variants. Room for 130 because (A1 | H1) is maximum with 129. */
    uint64_t state_castling[2][130];
} hash_zobrist_t;

typedef struct 
{
    uint64_t hash;
    int score;
    int depth;
    int type;
    int move;
} hash_node_t;

typedef struct
{
    uint64_t hash;
    int eval;
} hash_eval_t;

hash_zobrist_t *hash_zobrist;

void hash_init();
void hash_destroy();
void hash_set_tsize(int);
void hash_wipe();

uint64_t hash_make_zobrist(state_t*);

int hash_get_move(uint64_t);
int hash_probe(uint64_t, int, int, int, int*);

void hash_add_node(uint64_t, uint64_t, int, int, int);
hash_node_t *hash_get_node(uint64_t);

void hash_add_eval(uint64_t, int);
int hash_get_eval(uint64_t, int*);

#endif
