#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <string.h>
#include "state.h"
#include "search.h"
#include "eval.h"
#include "hash.h"
#include "plp.h"
#include "next.h"
#include "quiescence.h"
#include "make.h"

search_data_t search;

void search_go(state_t *state, int max_depth)
{
    assert(max_depth > 0);
    memset(&search, 0, sizeof(search_data_t));

    search_iterative(state, max_depth);
}

void search_iterative(state_t *state, int max_depth)
{
    int ply;
    for (ply = 1; ply <= max_depth; ++ply)
    {
        search.max_depth = ply;
        search_ab(state, ply, -INF, INF);
    }
}

int search_ab(state_t *state, int depth, int alpha, int beta)
{
    int ply = search.max_depth - depth;
    int *move, *end;
    int count = 0;
    int legal_move = 0;
    int best_move = 0;
    int hash_type = HASH_ALPHA;
    int moves[100];
    int score = 0;

    ++search.visited_nodes;

    if (state_is_repeating(state))
    {
        return 0;
    }

    if (hash_probe(state->zobrist, depth, alpha, beta, &score))
    {
        if (ply > 0)
        {
            return score;
        }
    }

    if (!depth)
    {
        return quiescence(state, search.max_depth, alpha, beta);
    }

    search.move_phase[ply] = PHASE_HASH;

    while (next_moves(state, moves, &count, ply, depth))
    {
        if (!count)
        {
            continue;
        }

        for (move = moves, end = moves + count; move < end; ++move)
        {
            make_move(state, *move, ply);

            if (move_is_attacked(state, state->king_idx[1 - state->turn], state->turn))
            {
                unmake_move(state, *move, ply);
                continue;
            }

            legal_move = 1;

            int eval = -search_ab(state, depth - 1, -beta, -alpha);
            unmake_move(state, *move, ply);

            if (eval > alpha)
            {
                if (eval >= beta)
                {
                    hash_add_node(state->zobrist, beta, depth, HASH_BETA, *move);
                    return beta;
                }

                alpha = eval;
                hash_type = HASH_EXACT;

                best_move = *move;

                if (ply == 0)
                {
                    search.best_score = eval;
                }

                search.pv[ply][ply] = *move;
                memcpy(&search.pv[ply][ply + 1], &search.pv[ply + 1][ply + 1], sizeof(int) * (search.max_depth - ply));
                /*
                int ii;
                for (ii = ply + 1; ii < search.max_depth; ++ii)
                {
                    search.pv[ply][ii] = search.pv[ply + 1][ii];
                }
                */
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
