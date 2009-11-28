#ifndef _NEXT_H
#define _NEXT_H

#include "state.h"

#define PHASE_HASH 0
#define PHASE_CAPTURES 1
#define PHASE_MOVES 2
#define PHASE_END 3

int next_moves(state_t*, int*, int*, int, int);

#endif
