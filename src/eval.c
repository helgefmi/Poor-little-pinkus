#include "eval.h"
#include "defines.h"
#include "state.h"
#if defined(__LP64__)
    #include "inline64.h"
#else
    #include "inline32.h"
#endif

static int eval_piece_values[] = {10, 30, 31, 50, 90, 500};

int eval_state(state_t *state)
{
    int ret = 0;

    int piece;
    for (piece = PAWN; piece <= KING; ++piece)
    {
        ret += eval_piece_values[piece] * PopCnt(state->pieces[state->turn][piece]);
    }

    return ret;
}
