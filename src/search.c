#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "search.h"
#include "eval.h"
#include "hash.h"
#include "plp.h"
#include "sort.h"
#include "next.h"

search_data_t search_data;

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

    int score = 0,
        ply = search_data.max_depth - depth;

    if (hash_probe(state->zobrist, depth, alpha, beta, &score))
    {
        ++search_data.cache_hits;
        if (ply > 0)
        {
            return score;
        }
    }
    else
    {
        ++search_data.cache_misses;
    }

    if (!depth)
    {
        return eval_state(state);
    }

    int moves[100];
    int count;

    search_data.move_phase[ply] = PHASE_HASH;
    int legal_move = 0, best_move = 0, hash_type = HASH_ALPHA;
    while (next_moves(state, moves, &count, ply))
    {
        if (!count)
        {
            continue;
        }

        if (depth > 1 && count > 1)
        {
            sort_moves(moves, count);
        }

        int i;
        for (i = 0; i < count; ++i)
        {
            move_make(state, moves[i], ply);

            if (move_is_attacked(state, state->king_idx[1 - state->turn], state->turn))
            {
                move_unmake(state, moves[i], ply);
                continue;
            }

            legal_move = 1;

            int eval = -search_ab(state, depth - 1, -beta, -alpha);
            move_unmake(state, moves[i], ply);

            if (eval >= beta)
            {
                hash_add_node(state->zobrist, beta, depth, HASH_BETA, moves[i]);
                return beta;
            }
            else if (eval > alpha)
            {
                alpha = eval;
                hash_type = HASH_EXACT;

                best_move = moves[i];

                if (ply == 0)
                {
                    search_data.best_score = eval;
                }

                search_data.pv[ply][ply] = moves[i];
                int ii;
                for (ii = ply + 1; ii < search_data.max_depth; ++ii)
                {
                    search_data.pv[ply][ii] = search_data.pv[ply + 1][ii];
                }
            }
        }
    }

    if (!legal_move)
    {
        int mate = move_is_attacked(state, state->king_idx[state->turn],  1 - state->turn);
        alpha = mate ? -INF + ply: -10;
    }
    else
    {
        hash_add_node(state->zobrist, alpha, depth, hash_type, best_move);
    }

    return alpha;
}
