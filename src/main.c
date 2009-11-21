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
    state_init_from_fen(&state, FEN_INIT);

    state_print(&state);
    test_divide(&state, 6);
    */

    test_perftsuite(4);

    cache_destroy();

    return 0;
}
