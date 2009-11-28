#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "next.h"
#include "move.h"
#include "search.h"
#include "hash.h"
#include "plp.h"

int next_moves(state_t *state, int *movebuf, int *count, int ply, int depth)
{
    register int hash_move;
    register int *move, *sortv, *end, swapped, tmp;
    static int sort_values[100];

    switch (search_data.move_phase[ply])
    {
        case PHASE_HASH:
            search_data.move_phase[ply] = PHASE_MOVES;
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

        case PHASE_CAPTURES:
            search_data.move_phase[ply] = PHASE_MOVES;
            return 1;

        case PHASE_MOVES:
            search_data.move_phase[ply] = PHASE_END;
            move_generate_moves(state, movebuf, count);

            if (*count < 2)
                return 1;

            hash_move = hash_get_move(state->zobrist);

            if (depth > 1)
            {
                /* Sort */
                end = movebuf + *count - 1;
                for (move = movebuf, sortv = sort_values; move <= end; ++move, ++sortv)
                {
                    if (*move == hash_move)
                    {
                        *sortv = -1024 * 1024;
                    }
                    else if (MoveCapture(*move) < 6)
                    {
                        *sortv = 512 + (MoveCapture(*move) - MovePiece(*move));
                    }
                    else
                    {
                        *sortv = -MovePiece(*move);
                    }
                }

                do
                {
                    swapped = 0;
                    for (move = movebuf, sortv = sort_values; move < end; ++move, ++sortv)
                    {
                        if (*sortv < *(sortv + 1))
                        {
                            tmp = *sortv;
                            *sortv = *(sortv + 1); 
                            *(sortv + 1) = tmp;
                            tmp = *move;
                            *move = *(move + 1); 
                            *(move + 1) = tmp;
                            swapped = 1;
                        }   
                    }
                } while (swapped);

                /* hash_move should be at the end of the move buffer at this point */
                if (hash_move)
                {
                    *count -= 1;
                }
            }

            return 1;

        case PHASE_END:
            return 0;
    }
    printf("invalid phase!\n");
    exit(1);
    return 0;
}
