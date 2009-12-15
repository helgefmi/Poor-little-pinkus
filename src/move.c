#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "move.h"
#include "util.h"
#include "cache.h"
#include "hash.h"
#include "search.h"
#include "plp.h"

void move_generate_moves(state_t *state, int *movebuf, int *count)
{
    int from, to;
    uint64_t pieces, moves;
    uint64_t not_occupied = ~state->occupied_both;
    uint64_t *my_pieces = state->pieces[state->turn];

    *count = 0;

    /* CASTLING */
    if (cached->castling_by_color[state->turn] & state->castling)
    {
        if ((state->castling & cached->castling_rooksq[state->turn][1]) &&
            !(state->occupied_both & cached->OO[state->turn]) &&
            !move_is_attacked(state, cached->OOsq[state->turn][0], Flip(state->turn)) &&
            !move_is_attacked(state, cached->OOsq[state->turn][1], Flip(state->turn)) &&
            !move_is_attacked(state, cached->OOsq[state->turn][2], Flip(state->turn)))
        {
            *movebuf++ = PackMove(state->king_idx[state->turn], cached->OOto[state->turn], KING, 7, 7);
            (*count)++;
        }

        if ((state->castling & cached->castling_rooksq[state->turn][0]) &&
            !(state->occupied_both & cached->OOO[state->turn]) &&
            !move_is_attacked(state, cached->OOOsq[state->turn][0], Flip(state->turn)) &&
            !move_is_attacked(state, cached->OOOsq[state->turn][1], Flip(state->turn)) &&
            !move_is_attacked(state, cached->OOOsq[state->turn][2], Flip(state->turn)))
        {
            *movebuf++ = PackMove(state->king_idx[state->turn], cached->OOOto[state->turn], KING, 7, 7);
            (*count)++;
        }
    }

    /* KNIGHT */
    for (pieces = my_pieces[KNIGHT]; pieces; ClearLow(pieces))
    {
        from = LSB(pieces);
        moves = cached->moves_knight[from] & not_occupied;

        for (; moves; ClearLow(moves))
        {
            to = LSB(moves);
            *movebuf++ = PackMove(from, to, KNIGHT, 7, 7);
            (*count)++;
        }
    }

    /* BISHOP */
    for (pieces = my_pieces[BISHOP]; pieces; ClearLow(pieces))
    {
        from = LSB(pieces);
        moves = cached->moves_bishop[from]
                & cached->n_directions[NW][LSB(cached->directions[NW][from] & state->occupied_both)]
                & cached->n_directions[NE][LSB(cached->directions[NE][from] & state->occupied_both)]
                & cached->n_directions[SE][MSB(cached->directions[SE][from] & state->occupied_both)]
                & cached->n_directions[SW][MSB(cached->directions[SW][from] & state->occupied_both)]
                & not_occupied;

        for (; moves; ClearLow(moves))
        {
            to = LSB(moves);
            *movebuf++ = PackMove(from, to, BISHOP, 7, 7);
            (*count)++;
        }
    }

    /* ROOK */
    for (pieces = my_pieces[ROOK]; pieces; ClearLow(pieces))
    {
        from = LSB(pieces);
        moves = cached->moves_rook[from]
                & cached->n_directions[NORTH][LSB(cached->directions[NORTH][from] & state->occupied_both)]
                & cached->n_directions[EAST][LSB(cached->directions[EAST][from] & state->occupied_both)]
                & cached->n_directions[SOUTH][MSB(cached->directions[SOUTH][from] & state->occupied_both)]
                & cached->n_directions[WEST][MSB(cached->directions[WEST][from] & state->occupied_both)]
                & not_occupied;

        for (; moves; ClearLow(moves))
        {
            to = LSB(moves);
            *movebuf++ = PackMove(from, to, ROOK, 7, 7);
            (*count)++;
        }
    }

    /* QUEEN */
    for (pieces = my_pieces[QUEEN]; pieces; ClearLow(pieces))
    {
        from = LSB(pieces);
        moves = (cached->moves_rook[from] | cached->moves_bishop[from])
                & cached->n_directions[NW][LSB(cached->directions[NW][from] & state->occupied_both)]
                & cached->n_directions[NE][LSB(cached->directions[NE][from] & state->occupied_both)]
                & cached->n_directions[SE][MSB(cached->directions[SE][from] & state->occupied_both)]
                & cached->n_directions[SW][MSB(cached->directions[SW][from] & state->occupied_both)]
                & cached->n_directions[NORTH][LSB(cached->directions[NORTH][from] & state->occupied_both)]
                & cached->n_directions[EAST][LSB(cached->directions[EAST][from] & state->occupied_both)]
                & cached->n_directions[SOUTH][MSB(cached->directions[SOUTH][from] & state->occupied_both)]
                & cached->n_directions[WEST][MSB(cached->directions[WEST][from] & state->occupied_both)]
                & not_occupied;

        for (; moves; ClearLow(moves))
        {
            to = LSB(moves);
            *movebuf++ = PackMove(from, to, QUEEN, 7, 7);
            (*count)++;
        }
    }

    /* PAWN */
    for (pieces = my_pieces[PAWN]; pieces; ClearLow(pieces))
    {
        from = LSB(pieces);
        moves = cached->moves_pawn_one[state->turn][from] & ~state->occupied_both & ~cached->promote[state->turn];
        if (moves)
        {
            moves = cached->moves_pawn_two[state->turn][from] & ~state->occupied_both;
        }

        for (; moves; ClearLow(moves))
        {
            to = LSB(moves);

            *movebuf++ = PackMove(from, to, PAWN, state->square[to], 7);
            (*count)++;
        }
    }


    /* KING */
    from = state->king_idx[state->turn];
    moves = cached->moves_king[from] & not_occupied;

    for (; moves; ClearLow(moves))
    {
        to = LSB(moves);
        *movebuf++ = PackMove(from, to, KING, 7, 7);
        (*count)++;
    }
}

void move_generate_tactical(state_t *state, int *movebuf, int *count)
{
    int from, to;
    uint64_t pieces, moves;
    uint64_t target = state->occupied[Flip(state->turn)];
    uint64_t *my_pieces = state->pieces[state->turn];

    *count = 0;

    /* PAWN PROMOTIONS */
    pieces = my_pieces[PAWN] & cached->promote_from[state->turn];
    for (; pieces; ClearLow(pieces))
    {
        from = LSB(pieces);
        moves = (cached->moves_pawn_one[state->turn][from] & ~state->occupied_both) |
                (cached->attacks_pawn[state->turn][from] & target);

        for (; moves; ClearLow(moves))
        {
            to = LSB(moves);

            *movebuf++ = PackMove(from, to, PAWN, state->square[to], QUEEN);
            (*count)++;
            *movebuf++ = PackMove(from, to, PAWN, state->square[to], ROOK);
            (*count)++;
            *movebuf++ = PackMove(from, to, PAWN, state->square[to], BISHOP);
            (*count)++;
            *movebuf++ = PackMove(from, to, PAWN, state->square[to], KNIGHT);
            (*count)++;
        }
    }

    /* PAWN ATTACKS */
    pieces = my_pieces[PAWN] & ~cached->promote_from[state->turn];
    for (; pieces; ClearLow(pieces))
    {
        from = LSB(pieces);
        moves = cached->attacks_pawn[state->turn][from] & target;

        for (; moves; ClearLow(moves))
        {
            to = LSB(moves);

            *movebuf++ = PackMove(from, to, PAWN, state->square[to], 7);
            (*count)++;
        }
    }

    /* EN PASSANT */
    if (state->en_passant)
    {
        to = LSB(state->en_passant);
        pieces = my_pieces[PAWN] & cached->attacked_by_pawn[state->turn][to];
        for (; pieces; ClearLow(pieces))
        {
            from = LSB(pieces);
            *movebuf++ = PackMove(from, to, PAWN, PAWN, 7);
            (*count)++;
        }
    }

    /* KNIGHT */
    for (pieces = my_pieces[KNIGHT]; pieces; ClearLow(pieces))
    {
        from = LSB(pieces);
        moves = cached->moves_knight[from] & target;

        for (; moves; ClearLow(moves))
        {
            to = LSB(moves);
            *movebuf++ = PackMove(from, to, KNIGHT, state->square[to], 7);
            (*count)++;
        }
    }

    /* BISHOP */
    for (pieces = my_pieces[BISHOP]; pieces; ClearLow(pieces))
    {
        from = LSB(pieces);
        if (cached->moves_bishop[from] & target)
        {
            moves = cached->moves_bishop[from]
                    & cached->n_directions[NW][LSB(cached->directions[NW][from] & state->occupied_both)]
                    & cached->n_directions[NE][LSB(cached->directions[NE][from] & state->occupied_both)]
                    & cached->n_directions[SE][MSB(cached->directions[SE][from] & state->occupied_both)]
                    & cached->n_directions[SW][MSB(cached->directions[SW][from] & state->occupied_both)]
                    & target;

            for (; moves; ClearLow(moves))
            {
                to = LSB(moves);
                *movebuf++ = PackMove(from, to, BISHOP, state->square[to], 7);
                (*count)++;
            }
        }
    }

    /* ROOK */
    for (pieces = my_pieces[ROOK]; pieces; ClearLow(pieces))
    {
        from = LSB(pieces);
        if (cached->moves_rook[from] & target)
        {
            moves = cached->moves_rook[from]
                    & cached->n_directions[NORTH][LSB(cached->directions[NORTH][from] & state->occupied_both)]
                    & cached->n_directions[EAST][LSB(cached->directions[EAST][from] & state->occupied_both)]
                    & cached->n_directions[SOUTH][MSB(cached->directions[SOUTH][from] & state->occupied_both)]
                    & cached->n_directions[WEST][MSB(cached->directions[WEST][from] & state->occupied_both)]
                    & target;

            for (; moves; ClearLow(moves))
            {
                to = LSB(moves);
                *movebuf++ = PackMove(from, to, ROOK, state->square[to], 7);
                (*count)++;
            }
        }
    }

    /* QUEEN */
    for (pieces = my_pieces[QUEEN]; pieces; ClearLow(pieces))
    {
        from = LSB(pieces);
        if (cached->moves_queen[from] & target)
        {
            moves = (cached->moves_rook[from] | cached->moves_bishop[from])
                    & cached->n_directions[NW][LSB(cached->directions[NW][from] & state->occupied_both)]
                    & cached->n_directions[NE][LSB(cached->directions[NE][from] & state->occupied_both)]
                    & cached->n_directions[SE][MSB(cached->directions[SE][from] & state->occupied_both)]
                    & cached->n_directions[SW][MSB(cached->directions[SW][from] & state->occupied_both)]
                    & cached->n_directions[NORTH][LSB(cached->directions[NORTH][from] & state->occupied_both)]
                    & cached->n_directions[EAST][LSB(cached->directions[EAST][from] & state->occupied_both)]
                    & cached->n_directions[SOUTH][MSB(cached->directions[SOUTH][from] & state->occupied_both)]
                    & cached->n_directions[WEST][MSB(cached->directions[WEST][from] & state->occupied_both)]
                    & target;

            for (; moves; ClearLow(moves))
            {
                to = LSB(moves);
                *movebuf++ = PackMove(from, to, QUEEN, state->square[to], 7);
                (*count)++;
            }
        }
    }

    /* KING */
    from = state->king_idx[state->turn];
    moves = cached->moves_king[from] & target;

    for (; moves; ClearLow(moves))
    {
        to = LSB(moves);
        *movebuf++ = PackMove(from, to, KING, state->square[to], 7);
        (*count)++;
    }
}

int move_is_attacked(state_t *state, int square, int attacker)
{
    /* Checks if a set of squares are currently attacked by an attackers pieces */

    if ((state->pieces[attacker][PAWN] & cached->attacked_by_pawn[attacker][square]) |
        (state->pieces[attacker][KNIGHT] & cached->moves_knight[square]) |
        (state->pieces[attacker][KING] & cached->moves_king[square]))
        return 1;

    uint64_t bishop_and_queen = state->pieces[attacker][BISHOP] | state->pieces[attacker][QUEEN];

    if (cached->moves_bishop[square] & bishop_and_queen)
    {
        uint64_t nw_hits = state->occupied_both & cached->directions[NW][square];
        if ((nw_hits & -nw_hits) & bishop_and_queen)
            return 1;

        uint64_t ne_hits = state->occupied_both & cached->directions[NE][square];
        if ((ne_hits & -ne_hits) & bishop_and_queen)
            return 1;

        uint64_t sw_hits = state->occupied_both & cached->directions[SW][square];
        if (sw_hits && ((1ull << MSB(sw_hits)) & bishop_and_queen))
            return 1;

        uint64_t se_hits = state->occupied_both & cached->directions[SE][square];
        if (se_hits && ((1ull << MSB(se_hits)) & bishop_and_queen))
            return 1;
    }

    uint64_t rook_and_queen = state->pieces[attacker][ROOK] | state->pieces[attacker][QUEEN];

    if (cached->moves_rook[square] & rook_and_queen)
    {
        uint64_t north_hits = state->occupied_both & cached->directions[NORTH][square];
        if ((north_hits & -north_hits) & rook_and_queen)
            return 1;

        uint64_t east_hits = state->occupied_both & cached->directions[EAST][square];
        if ((east_hits & -east_hits) & rook_and_queen)
            return 1;

        uint64_t south_hits = state->occupied_both & cached->directions[SOUTH][square];
        if (south_hits && ((1ull << MSB(south_hits)) & rook_and_queen))
            return 1;

        uint64_t west_hits = state->occupied_both & cached->directions[WEST][square];
        if (west_hits && ((1ull << MSB(west_hits)) & rook_and_queen))
            return 1;
    }

    return 0;
}

void move_sort_captures(int *movebuf, int count, int hash_move)
{
    int *move, *end, *sortv, tmp, swapped;
    static int sort_values[100];

    end = movebuf + count - 1;
    for (move = movebuf, sortv = sort_values; move <= end; ++move, ++sortv)
    {
        if (*move == hash_move)
        {
            *sortv = -1024 * 1024;
        }
        else if (MoveCapture(*move) < 6)
        {
            *sortv = 512 * MoveCapture(*move) - MovePiece(*move);
        }
        else
        {
            *sortv = 1024 * 1024;
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
}

#ifdef USE_HISTORY
void move_sort_moves(int *movebuf, int count)
{
    int *move, *end, tmp, swapped;

    end = movebuf + count - 1;

    do
    {
        swapped = 0;
        for (move = movebuf; move < end; ++move)
        {
            if (search.history[*move & 0x7fff] < search.history[*(move + 1) & 0x7fff])
            {
                tmp = *move;
                *move = *(move + 1); 
                *(move + 1) = tmp;
                swapped = 1;
            }   
        }
    } while (swapped);
}
#endif
