#ifndef _EVAL_H
#define _EVAL_H

#include "state.h"

int eval_state(state_t*);
int eval_quick(state_t*);
extern int eval_piece_values[6];
extern int eval_real_pvalues[6];

#endif
