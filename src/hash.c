#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "hash.h"
#include "defines.h"
#if defined(__LP64__)
    #include "inline64.h"
#else
    #include "inline32.h"
#endif

static hash_node_t *hash_table;
static int _hash_table_size;

static inline uint64_t _rand64()
{
    return rand() ^ ((uint64_t)rand() << 15)
            ^ ((uint64_t)rand() << 30) ^ ((uint64_t)rand() << 45)
            ^ ((uint64_t)rand() << 60);
}

static void _init_zobrist()
{
    /* Initializes the random values we'll use for making zobrist keys.
     * Different srand seeds will give different results */

    memset(hash_zobrist, 0, sizeof(hash_zobrist_t));

    int piece, color, idx;
    for (color = WHITE; color <= BLACK; ++color)
    {
        for (piece = PAWN; piece <= KING; ++piece)
        {
            for (idx = 0; idx < 64; ++idx)
            {
                hash_zobrist->pieces[color][piece][idx] = _rand64();
            }
        }
    }

    for (idx = 0; idx < 64; ++idx)
    {
        hash_zobrist->en_passant[idx] = _rand64();
        hash_zobrist->castling[idx] = _rand64();
    }

    hash_zobrist->rook_castling[WHITE][4][2] = hash_zobrist->pieces[WHITE][ROOK][0] | hash_zobrist->pieces[WHITE][ROOK][3];
    hash_zobrist->rook_castling[WHITE][4][6] = hash_zobrist->pieces[WHITE][ROOK][5] | hash_zobrist->pieces[WHITE][ROOK][7];

    hash_zobrist->rook_castling[BLACK][7* 8 + 4][7 * 8 + 2] = hash_zobrist->pieces[BLACK][ROOK][7 * 8 + 0] | hash_zobrist->pieces[BLACK][ROOK][7 * 8 + 3];
    hash_zobrist->rook_castling[BLACK][7* 8 + 4][7 * 8 + 6] = hash_zobrist->pieces[BLACK][ROOK][7 * 8 + 5] | hash_zobrist->pieces[BLACK][ROOK][7 * 8 + 7];

    hash_zobrist->state_castling[WHITE][1] = hash_zobrist->castling[0];
    hash_zobrist->state_castling[WHITE][128] = hash_zobrist->castling[7];
    hash_zobrist->state_castling[WHITE][129] = hash_zobrist->castling[0] | hash_zobrist->castling[7];

    hash_zobrist->state_castling[BLACK][1] = hash_zobrist->castling[8 * 7 + 0];
    hash_zobrist->state_castling[BLACK][128] = hash_zobrist->castling[8 * 7 + 7];
    hash_zobrist->state_castling[BLACK][129] = hash_zobrist->castling[8 * 7 + 0] | hash_zobrist->castling[8 * 7 + 7];

    hash_zobrist->turn = _rand64();
}


void hash_init()
{
    assert(!hash_table);

    hash_zobrist = malloc(sizeof(hash_zobrist_t));
    _init_zobrist();
}

void hash_destroy()
{
    if (hash_table)
    {
        free(hash_table);
    }

    free(hash_zobrist);
}

void hash_set_tsize(int size)
{
    if (hash_table)
    {
        free(hash_table);
    }

    _hash_table_size = size;
    hash_table = malloc(sizeof(hash_node_t) * _hash_table_size);
    memset(hash_table, 0, sizeof(hash_node_t) * _hash_table_size);
}

int hash_probe(uint64_t zobrist_key, int depth, int alpha, int beta, int *score)
{
    hash_node_t *entry = hash_get_node(zobrist_key);
    if (!entry || entry->depth < depth || entry->hash != zobrist_key)
    {
        return 0;
    }

    if (entry->type == HASH_EXACT)
    {
        *score = entry->score;
        return 1;
    }
    else if (entry->type == HASH_ALPHA)
    {
        if (entry->score <= alpha)
        {
            *score = alpha;
            return 1;
        }
    }
    else if (entry->type == HASH_BETA)
    {
        if (entry->score >= beta)
        {
            *score = beta;
            return 1;
        }
    }

    return 0;
}

void hash_add_node(uint64_t zobrist_key, uint64_t score, int depth, int type)
{
    if (!_hash_table_size)
    {
        return;
    }

    int idx = zobrist_key % _hash_table_size;

    hash_table[idx].hash = zobrist_key;
    hash_table[idx].depth = depth;
    hash_table[idx].score = score;
    hash_table[idx].type = type;
}

hash_node_t *hash_get_node(uint64_t zobrist_key)
{
    if (!_hash_table_size)
    {
        return NULL;
    }

    /* The returned node must be checked to see if the zobrist keys are matching. */
    int idx = zobrist_key % _hash_table_size;
    return &hash_table[idx];
}


uint64_t hash_make_zobrist(state_t *state)
{
    /* Makes a zobrist key from scratch, given a state.
     * Should only be called once, when initializing a new state */
    uint64_t ret = 0;

    int color, piece;
    for (color = WHITE; color <= BLACK; ++color)
    {
        for (piece = PAWN; piece <= KING; ++piece)
        {
            uint64_t pieces = state->pieces[color][piece];
            while (pieces)
            {
                int piece_idx = LSB(pieces);
                pieces &= pieces - 1;

                ret ^= hash_zobrist->pieces[color][piece][piece_idx];
            }
        }
    }

    if (state->en_passant)
    {
        int en_passant_idx = LSB(state->en_passant);
        ret ^= hash_zobrist->en_passant[en_passant_idx];
    }


    uint64_t castling = state->castling;
    while (castling)
    {
        int castling_idx = LSB(castling & -castling);
        castling &= castling - 1;

        ret ^= hash_zobrist->castling[castling_idx];
    }

    if (state->turn)
    {
        ret ^= hash_zobrist->turn;
    }

    return ret;
}

void hash_wipe()
{
    memset(hash_table, 0, sizeof(hash_table));
}
