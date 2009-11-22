#ifndef _HASHING_H
#define _HASHING_H

#include <stdint.h>
#include "state.h"

typedef struct {
    uint64_t pieces[2][6][64];
    uint64_t turn, en_passant[64], castling[64];
} hash_zobrist_t;

typedef struct 
{
    uint64_t hash;
    uint64_t score;
    int depth;
} hash_node_t;

hash_zobrist_t *hash_zobrist;

void hash_init(int);
void hash_destroy();

uint64_t hash_make_zobrist(state_t*);

void hash_add_node(uint64_t, uint64_t, int);
hash_node_t *hash_get_node(uint64_t);

#endif
