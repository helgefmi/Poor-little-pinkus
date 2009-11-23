#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "search.h"
#include "eval.h"
#include "defines.h"

int cmp_sort_captures(const void* a, const void* b)
{
    const move_t* move_a = (const move_t*) a;
    const move_t* move_b = (const move_t*) b;
    return ((int)move_b->capture - (int)move_b->from_piece) - ((int)move_a->capture - (int)move_a->from_piece);
}

void search_go(state_t *state, int max_depth)
{
    assert(max_depth > 0);

    if (search_best_move)
    {
        free(search_best_move);
    }

    search_best_move = malloc(sizeof(search_best_move_t));
    memset(search_best_move, 0, sizeof(search_best_move_t));

    search_ab(state, 0, max_depth, -99999999, 99999999);
}

int search_ab(state_t *state, int depth, int max_depth, int alpha, int beta)
{
    if (depth == max_depth)
    {
        return eval_state(state);
    }

    move_t moves[100];
    int count = 0;

    move_generate_moves(state, moves, &count);
    qsort(moves, count, sizeof(move_t), cmp_sort_captures);

    int i, have_moved = 0;
    for (i = 0; i < count; ++i)
    {
        move_make(state, &moves[i]);

        if (!move_is_attacked(state, state->pieces[1 - state->turn][KING], state->turn))
        {
            have_moved = 1;
            int eval = -search_ab(state, depth + 1, max_depth, -beta, -alpha);

            if (eval >= beta)
            {
                move_unmake(state, &moves[i]);
                return beta;
            }

            if (eval > alpha)
            {
                alpha = eval;
                if (depth == 0)
                {
                    memcpy(&search_best_move->move, &moves[i], sizeof(move_t));
                    search_best_move->score = eval;
                    search_best_move->depth = max_depth;
                }
            }
        }

        move_unmake(state, &moves[i]);
    }

    if (!have_moved)
    {
        int mate = move_is_attacked(state, state->pieces[state->turn][KING],  1 - state->turn);
        return mate ? -10000000 : -50;
    }

    return alpha;
}
