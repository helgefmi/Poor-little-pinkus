#ifndef _TEST_H
#define _TEST_H

#include "state.h"

uint64_t test_perft_rec(state_t*, int, int);
void test_perft(state_t*, int, int);
void test_perftsuite(int);

#endif
