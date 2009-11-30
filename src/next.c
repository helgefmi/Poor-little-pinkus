#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "next.h"
#include "move.h"
#include "search.h"
#include "hash.h"
#include "util.h"
#include "plp.h"

int next_moves(state_t *state, int *movebuf, int *count, int ply, int depth)
{
    int hash_move;

    switch (search.move_phase[ply])
    {
        case PHASE_HASH:
            search.move_phase[ply] = PHASE_TACTICAL;
            hash_move = hash_get_move(state->zobrist);
            if (hash_move)
            {
                *movebuf = hash_move;
                *count = 1;
            }
            else
            {
                *count = 0;
            }
            return 1;

        case PHASE_TACTICAL:
            search.move_phase[ply] = PHASE_KILLER1;
            move_generate_tactical(state, movebuf, count);

            if (*count < 2)
                return 1;

            if (depth > 1)
            {
                hash_move = hash_get_move(state->zobrist);
                move_sort_captures(movebuf, *count, hash_move);

                /* hash_move should be at the end of the move buffer at this point */
                if (hash_move)
                {
                    *count -= 1;
                }
            }
            return 1;

        case PHASE_KILLER1:
            search.move_phase[ply] = PHASE_KILLER2;
            if (Killer1(ply) && util_legal_killer(state, Killer1(ply)))
            {
                *movebuf = Killer1(ply);
                *count = 1;
            }
            else
            {
                *count = 0;
            }
            return 1;

        case PHASE_KILLER2:
            search.move_phase[ply] = PHASE_MOVES;
            if (Killer2(ply) && util_legal_killer(state, Killer2(ply)))
            {
                *movebuf = Killer2(ply);
                *count = 1;
            }
            else
            {
                *count = 0;
            }
            return 1;

        case PHASE_MOVES:
            search.move_phase[ply] = PHASE_END;
            move_generate_moves(state, movebuf, count);
            return 1;

        case PHASE_END:
            return 0;
    }
    printf("invalid phase!\n");
    exit(1);
    return 0;
}