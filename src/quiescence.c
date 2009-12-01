#include "search.h"
#include "quiescence.h"
#include "eval.h"
#include "hash.h"
#include "move.h"
#include "state.h"
#include "plp.h"
#include "cache.h"
#include "make.h"
#include "timectrl.h"

int quiescence(state_t *state, int ply, int alpha, int beta)
{
    int *move, *end, eval;
    int moves[100], count, test;

    if (timectrl_should_halt())
        return 0;

    ++search.qs_visited_nodes;

    /* Handle repetition */
    if (state_is_repeating(state))
    {
        return 0;
    }

    /* Stand pat value */
    eval = eval_state(state);

    /* If it's hopeless for alpha, we can exit early */
    test = eval + eval_piece_values[QUEEN];
    if (state->pieces[state->turn][PAWN] & cached->promote_from[state->turn])
    {
        test += eval_piece_values[QUEEN];
    }

    if (test < alpha)
    {
        return alpha;
    }

    if (eval > alpha)
    {
        if (eval >= beta)
            return beta;

        alpha = eval;
    }

    move_generate_tactical(state, moves, &count);
    move_sort_captures(moves, count, 0);

    for (move = moves, end = moves + count; move < end; ++move)
    {
        make_move(state, *move, ply);

        if (move_is_attacked(state, state->king_idx[Flip(state->turn)], state->turn))
        {
            unmake_move(state, *move, ply);
            continue;
        }

        eval = -quiescence(state, ply + 1, -beta, -alpha);
        unmake_move(state, *move, ply);

        if (eval > alpha)
        {
            if (eval >= beta)
                return beta;

            alpha = eval;
        }
    }

    return alpha;
}
