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
    util_init();

    /*
    state_t state;
    state_init_from_fen(&state, "r3k2r/p1ppqpb1/bn2pnp1/3PN3/Pp2P3/2N2Q1p/1PPBBPPP/R3K2R b KQkq a3 0 1");

    state_print(&state);
    test_divide(&state, 1);
    */

    test_perftsuite(4);

    cache_destroy();

    return 0;
}
