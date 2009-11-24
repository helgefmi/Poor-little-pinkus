#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "search.h"
#include "eval.h"
#include "defines.h"
#include "hash.h"

search_data_t search_data;

int cmp_sort_captures(const void* a, const void* b)
{
    return ((const move_t *)b)->move_score - ((const move_t *)a)->move_score;
}

void search_go(state_t *state, int max_depth)
{
    assert(max_depth > 0);
    memset(&search_data, 0, sizeof(search_data_t));
    search_iterative(state, max_depth);
}

int search_iterative(state_t *state, int max_depth)
{
    search_data.max_depth = max_depth;
    return search_ab(state, max_depth, -INF, INF);
}

int search_ab(state_t *state, int depth, int alpha, int beta)
{
    ++search_data.visited_nodes;

    int score = 0;
    if (hash_probe(state->zobrist, depth, alpha, beta, &score))
    {
        ++search_data.cache_hits;
        return score;
    }
    else
    {
        ++search_data.cache_misses;
    }

    if (!depth)
    {
        return eval_state(state);
    }

    int hash_type = HASH_ALPHA,
        ply = search_data.max_depth - depth;

    move_t moves[100];
    int count = 0;
    move_generate_moves(state, moves, &count);

    assert(count >= 0);
    qsort(moves, count, sizeof(move_t), cmp_sort_captures);

    int i, legal_move = 0;
    for (i = 0; i < count; ++i)
    {
        move_make(state, &moves[i]);

        if (move_is_attacked(state, state->pieces[1 - state->turn][KING], state->turn))
        {
            move_unmake(state, &moves[i]);
            continue;
        }

        legal_move = 1;

        int eval = -search_ab(state, depth - 1, -beta, -alpha);
        move_unmake(state, &moves[i]);

        if (eval >= beta)
        {
            hash_add_node(state->zobrist, beta, depth, HASH_BETA);
            return beta;
        }
        else if (eval > alpha)
        {
            alpha = eval;
            hash_type = HASH_EXACT;

            memcpy(&search_data.pv[ply].move, &moves[i], sizeof(move_t));
            search_data.pv[ply].score = eval;
            search_data.pv[ply].depth = depth;
        }
    }

    if (!legal_move)
    {
        int mate = move_is_attacked(state, state->pieces[state->turn][KING],  1 - state->turn);
        alpha = mate ? -INF + ply: -10;
    }
    else
    {
        hash_add_node(state->zobrist, alpha, depth, hash_type);
    }

    return alpha;
}
