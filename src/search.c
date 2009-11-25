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
    const move_t *move_a = (const void*) a;
    const move_t *move_b = (const void*) b;

    return (1024 * -(search_data.best_move_id == move_a->move_id)) + 
           (1024 *  (search_data.best_move_id == move_b->move_id)) +
           move_b->move_score - move_a->move_score;
}

void search_go(state_t *state, int max_depth)
{
    assert(max_depth > 0);
    memset(&search_data, 0, sizeof(search_data_t));

    search_iterative(state, max_depth);
}

void search_iterative(state_t *state, int max_depth)
{
    int ply;
    for (ply = 1; ply <= max_depth; ++ply)
    {
        search_data.max_depth = ply;
        search_ab(state, ply, -INF, INF);
    }
}

int search_ab(state_t *state, int depth, int alpha, int beta)
{
    ++search_data.visited_nodes;

    int score = 0;
    search_data.best_move_id = -1;
    if (hash_probe(state->zobrist, depth, alpha, beta, &score, &search_data.best_move_id))
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

    qsort(moves, count, sizeof(move_t), cmp_sort_captures);

    int i, legal_move = 0, best_move_id = -1;
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
            hash_add_node(state->zobrist, beta, depth, HASH_BETA, moves[i].move_id);
            return beta;
        }
        else if (eval > alpha)
        {
            alpha = eval;
            hash_type = HASH_EXACT;

            best_move_id = moves[i].move_id;

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
        hash_add_node(state->zobrist, alpha, depth, hash_type, best_move_id);
    }

    return alpha;
}
