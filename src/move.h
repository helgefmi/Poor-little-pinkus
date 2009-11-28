#ifndef _MOVE_H
#define _MOVE_H

#include "state.h"

#define MSCORE_DEFAULT 0
#define MSCORE_CAPTURE 10
#define MSCORE_PROMOTION 100

void move_generate_moves(state_t*, int*, int*);
void move_generate_tactical(state_t*, int*, int*);

int move_is_attacked(state_t*, int, int);

void move_make(state_t*, int, int);
void move_unmake(state_t*, int, int);

#endif
