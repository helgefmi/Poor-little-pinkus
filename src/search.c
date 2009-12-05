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
#include "timectrl.h"

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
        search_ab(state, depth, 0, -INF, INF, 0);

        if (timecontrol.verbose)
            timectrl_notify_uci();

        if (timectrl_should_halt())
            break;

        /* Since we're using Iterative Deepening, a mate result will always be the fastest mate. */
        if (Abs(search.best_score) >= INF - MAX_DEPTH)
            break;
    }
}

int search_ab(state_t *state, int depth, int ply, int alpha, int beta, int can_null)
{
    int *move, *end;
    int count = 0;
    int legal_move = 0;
    int best_move = 0;
    int hash_type;
    int moves[100];
    int score = 0;
    int in_check = move_is_attacked(state, state->king_idx[state->turn], Flip(state->turn));

    if (timectrl_should_halt())
        return 0;

    ++search.visited_nodes;

    /* Repetition */
    if (state_is_repeating(state))
        return 0;

    /* Hash probe */
    if ((hash_type = hash_probe(state->zobrist, depth, alpha, beta, &score)))
    {
        if (hash_type == HASH_EXACT)
        {
            search.pv[ply][ply] = hash_get_move(state->zobrist);
            memcpy(&search.pv[ply][ply + 1], &search.pv[ply + 1][ply + 1], sizeof(int) * depth);
        }
        /* If our previous PV is currently failing, and the computer is struggeling with finding a better PV,
         * we need to copy the old best move in case we can't find a solution */
        else if (!search.pv[ply][ply])
        {
            search.pv[ply][ply] = hash_get_move(state->zobrist);
        }

        return score;
    }

    /* Check extension */
    if (in_check)
        depth += 1;

    /* Evaluate */
    if (!depth)
        return quiescence(state, ply + 1, alpha, beta);

    /* Null move */
    if (can_null && depth > 2 && !search.in_endgame && !in_check)
    {
        int R = 2;
        if (depth > 6)
            R = 3;

        make_null_move(state, ply);
        int eval = -search_ab(state, depth - 1 - R, ply + 1, -beta, -beta + 1, 0);
        unmake_null_move(state, ply);

        if (eval >= beta)
            return beta;
    }

    /* Move generation */
    hash_type = HASH_ALPHA;
    search.move_phase[ply] = PHASE_HASH;
    while (next_moves(state, moves, &count, ply, depth))
    {
        if (!count)
            continue;

        for (move = moves, end = moves + count; move < end; ++move)
        {
            make_move(state, *move, ply);

            /* Legal position ? */
            if (move_is_attacked(state, state->king_idx[Flip(state->turn)], state->turn))
            {
                unmake_move(state, *move, ply);
                continue;
            }

            legal_move = 1;

            int eval = -search_ab(state, depth - 1, ply + 1, -beta, -alpha, 1);
            unmake_move(state, *move, ply);

            if (timectrl_should_halt())
                break;


            if (eval > alpha)
            {
                /* Fail high ? */
                if (eval >= beta)
                {
                    /* Add hash */
                    hash_add_node(state->zobrist, beta, depth, HASH_BETA, *move);

                    /* Add killer */
                    if (MoveCapture(*move) > 5 && MovePromote(*move) > 5 && *move != search.killers[ply][0])
                    {
                        search.killers[ply][1] = search.killers[ply][0];
                        search.killers[ply][0] = *move;
                    }
                    return beta;
                }

                /* PV node */
                alpha = eval;
                hash_type = HASH_EXACT;

                best_move = *move;

                if (ply == 0)
                    search.best_score = eval;

                /* Killer */
                if (MoveCapture(*move) > 5 && MovePromote(*move) > 5 && *move != search.killers[ply][0])
                {
                    search.killers[ply][1] = search.killers[ply][0];
                    search.killers[ply][0] = best_move;
                }

                /* For PV extraction */
                search.pv[ply][ply] = *move;
                memcpy(&search.pv[ply][ply + 1], &search.pv[ply + 1][ply + 1], sizeof(int) * depth);
            }
        }
    }

    if (!legal_move)
        alpha = in_check ? -INF + ply: -10;
    else
        hash_add_node(state->zobrist, alpha, depth, hash_type, best_move);

    return alpha;
}
