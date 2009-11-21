#include <stdlib.h>
#include <stdio.h>
#include "defines.h"
#include "cache.h"
#include "state.h"
#include "test.h"
#include "move.h"
#include "util.h"

int main()
{
    cache_init();

    /*
    state_t state;
    state_init_from_fen(&state, "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1");

    state_print(&state);
    test_divide(&state, 3);
    */

    test_perftsuite(4);

    cache_destroy();

    return 0;
}
