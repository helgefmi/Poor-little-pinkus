#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "search.h"
#include "eval.h"
#include "defines.h"

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
    return search_ab(state, 0, max_depth, -INF, INF, 1);
}

int search_ab(state_t *state, int depth, int max_depth, int alpha, int beta, int is_pv)
{
    ++search_data.visited_nodes;

    if (depth == max_depth)
    {
        return eval_state(state);
    }

    move_t moves[100];
    int count = 0;

    move_generate_moves(state, moves, &count);

    assert(count >= 0);

    qsort(moves, count, sizeof(move_t), cmp_sort_captures);

    int i, legal_move = 0;
    for (i = 0; i < count; ++i)
    {
        move_make(state, &moves[i]);

        if (!move_is_attacked(state, state->pieces[1 - state->turn][KING], state->turn))
        {
            legal_move = 1;

            int eval = -search_ab(state, depth + 1, max_depth, -beta, -alpha, is_pv);

            if (eval >= beta)
            {
                move_unmake(state, &moves[i]);
                return beta;
            }
            else if (eval > alpha)
            {
                alpha = eval;

                memcpy(&search_data.pv[depth].move, &moves[i], sizeof(move_t));
                search_data.pv[depth].score = eval;
                search_data.pv[depth].depth = max_depth;
            }
        }

        move_unmake(state, &moves[i]);
    }

    if (!legal_move)
    {
        int mate = move_is_attacked(state, state->pieces[state->turn][KING],  1 - state->turn);
        return mate ? -INF + depth: -10;
    }

    return alpha;
}
