#include "state.h"
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
    int moves[64], count, test;

    if (timectrl_should_halt())
        return 0;

    ++search.qs_visited_nodes;

#ifdef USE_REPETITION
    /* Handle repetition */
    if (state_is_repeating(state))
        return 0;
#endif

    /* Stand pat value */
    eval = eval_state(state);

    /* If it's hopeless for alpha, we can exit early */
    test = eval + eval_piece_values[QUEEN];
    if (state->pieces[state->turn][PAWN] & cached->promote_from[state->turn])
        test += eval_piece_values[QUEEN];

    if (test < alpha)
        return alpha;

    if (eval > alpha)
    {
        if (eval >= beta)
            return beta;

        alpha = eval;
    }

    move_generate_tactical(state, moves, &count);
    if (count > 1)
    {
        move_sort_captures(moves, count, hash_get_move(state->zobrist));
    }

    for (move = moves, end = moves + count; move < end; ++move)
    {
        if ((eval_real_pvalues[MovePiece(*move)] > eval_real_pvalues[MoveCapture(*move)]) &&
            (state_see(state, *move) < 0))
            continue;

        make_move(state, *move, ply);

        if (move_is_attacked(state, state->king_idx[Flip(state->turn)], state->turn))
        {
            unmake_move(state, *move, ply);
            continue;
        }

        eval = -quiescence(state, ply + 1, -beta, -alpha);
        unmake_move(state, *move, ply);

        if (timectrl_should_halt())
            break;

        if (eval > alpha)
        {
            if (eval >= beta)
                return beta;

            alpha = eval;
        }
    }

    return alpha;
}
