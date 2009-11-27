#include "eval.h"
#include "plp.h"
#include "state.h"

static int eval_piece_values[6] = {10, 30, 31, 50, 90, 500};

int eval_state(state_t *state)
{
    int ret = 0;

    int piece;
    for (piece = PAWN; piece <= KING; ++piece)
    {
        ret += eval_piece_values[piece] * PopCnt(state->pieces[state->turn][piece]);
        ret -= eval_piece_values[piece] * PopCnt(state->pieces[1 - state->turn][piece]);
    }

    return ret;
}
