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
    int depth;
    for (depth = 1; depth <= max_depth; ++depth)
    {
        search.max_depth = depth;
        search.can_nullmove[0] = 0;
        search.can_nullmove[1] = 1;
        search_ab(state, depth, 0, -INF, INF);

        /* Since we're using Iterative Deepening, a mate result will always be the fastest mate. */
        if (Abs(search.best_score) >= INF - MAX_DEPTH)
            return;
    }
}

int search_ab(state_t *state, int depth, int ply, int alpha, int beta)
{
    int *move, *end;
    int count = 0;
    int legal_move = 0;
    int best_move = 0;
    int hash_type = HASH_ALPHA;
    int moves[100];
    int score = 0;
    int in_check = move_is_attacked(state, state->king_idx[state->turn], Flip(state->turn));

    ++search.visited_nodes;

    /* Repetition */
    if (state_is_repeating(state))
        return 0;

    /* Hash probe */
    if (hash_probe(state->zobrist, depth, alpha, beta, &score))
    {
        if (ply > 0)
            return score;
    }

    /* Check extension */
    if (in_check)
        depth += 1;

    /* Evaluate */
    if (!depth)
        return quiescence(state, ply + 1, alpha, beta);

    /* Null move */
    if (search.can_nullmove[ply] && depth > 2 && !search.in_endgame && !in_check)
    {
        search.can_nullmove[ply + 1] = 0;

        make_null_move(state, ply);
        int eval = -search_ab(state, depth - 3, ply + 1, -beta, -beta + 1);
        unmake_null_move(state, ply);

        if (eval >= beta)
            return beta;
    }
    else
    {
        /* To prevent double null moves */
        search.can_nullmove[ply + 1] = 1;
    }

    /* Move generation */
    search.move_phase[ply] = PHASE_HASH;
    while (next_moves(state, moves, &count, ply, depth))
    {
        if (!count)
            continue;

        for (move = moves, end = moves + count; move < end; ++move)
        {
            make_move(state, *move, ply);

            if (move_is_attacked(state, state->king_idx[Flip(state->turn)], state->turn))
            {
                unmake_move(state, *move, ply);
                continue;
            }

            legal_move = 1;

            int eval = -search_ab(state, depth - 1, ply + 1, -beta, -alpha);
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
                    search.best_score = eval;

                search.pv[ply][ply] = *move;
                memcpy(&search.pv[ply][ply + 1], &search.pv[ply + 1][ply + 1], sizeof(int) * depth);
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
        alpha = in_check ? -INF + ply: -10;
    else
        hash_add_node(state->zobrist, alpha, depth, hash_type, best_move);

    return alpha;
}
