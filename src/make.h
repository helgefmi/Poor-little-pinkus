#ifndef _MAKE_H
#define _MAKE_H

#include "state.h"

void make_move(state_t*, int, int);
void unmake_move(state_t*, int, int);

void make_null_move(state_t*, int);
void unmake_null_move(state_t*, int);

#endif
