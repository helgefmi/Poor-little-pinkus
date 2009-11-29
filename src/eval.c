#include "eval.h"
#include "plp.h"
#include "state.h"

int eval_piece_values[6] = {10, 30, 31, 50, 90, 500};

static int eval_material(state_t *state)
{
    int ret = 0;
    int *piecev = &eval_piece_values[0], *end = &eval_piece_values[0] + 6;
    uint64_t *mypiece = state->pieces[state->turn], *oppiece = state->pieces[Flip(state->turn)];

    for (; piecev < end; ++piecev, ++mypiece, ++oppiece)
    {
        ret += *piecev * (PopCnt(*mypiece) - PopCnt(*oppiece));
    }

    return ret;
}

int eval_state(state_t *state)
{
    int ret = eval_material(state);
    return ret;
}
