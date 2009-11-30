#ifndef _NEXT_H
#define _NEXT_H

#include "state.h"

#define PHASE_HASH 0
#define PHASE_TACTICAL 1
#define PHASE_KILLER1 2
#define PHASE_KILLER2 3
#define PHASE_MOVES 4
#define PHASE_END 5

int next_moves(state_t*, int*, int*, int, int);

#endif
