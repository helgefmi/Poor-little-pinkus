#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "move.h"
#include "util.h"
#include "cache.h"
#include "hash.h"
#include "plp.h"

void move_generate_moves(state_t *state, int *movebuf, int *count)
{
    register int from, to;
    register uint64_t pieces, moves;
    register uint64_t not_occupied = ~state->occupied[state->turn];
    register uint64_t *my_pieces = state->pieces[state->turn];

    /* CASTLING */

    if (cached->castling_by_color[state->turn] & state->castling)
    {
        if ((state->castling & cached->castling_rooksq[state->turn][1]) &&
            !(state->occupied_both & cached->OO[state->turn]) &&
            !move_is_attacked(state, cached->OOsq[state->turn][0], 1 - state->turn) &&
            !move_is_attacked(state, cached->OOsq[state->turn][1], 1 - state->turn) &&
            !move_is_attacked(state, cached->OOsq[state->turn][2], 1 - state->turn))
        {
            *movebuf++ = PackMove(state->king_idx[state->turn], cached->OOto[state->turn], KING, -1, -1);
            (*count)++;
        }

        if ((state->castling & cached->castling_rooksq[state->turn][0]) &&
            !(state->occupied_both & cached->OOO[state->turn]) &&
            !move_is_attacked(state, cached->OOOsq[state->turn][0], 1 - state->turn) &&
            !move_is_attacked(state, cached->OOOsq[state->turn][1], 1 - state->turn) &&
            !move_is_attacked(state, cached->OOOsq[state->turn][2], 1 - state->turn))
        {
            *movebuf++ = PackMove(state->king_idx[state->turn], cached->OOOto[state->turn], KING, -1, -1);
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
            *movebuf++ = PackMove(from, to, KNIGHT, state->square[to], -1);
            (*count)++;
        }
    }

    /* BISHOP */
    for (pieces = my_pieces[BISHOP]; pieces; ClearLow(pieces))
    {
        from = LSB(pieces);
        moves = cached->moves_bishop[from]
                & ~cached->directions[NW][LSB(cached->directions[NW][from] & state->occupied_both)]
                & ~cached->directions[NE][LSB(cached->directions[NE][from] & state->occupied_both)]
                & ~cached->directions[SE][MSB(cached->directions[SE][from] & state->occupied_both)]
                & ~cached->directions[SW][MSB(cached->directions[SW][from] & state->occupied_both)]
                & not_occupied;

        for (; moves; ClearLow(moves))
        {
            to = LSB(moves);
            *movebuf++ = PackMove(from, to, BISHOP, state->square[to], -1);
            (*count)++;
        }
    }

    /* ROOK */
    for (pieces = my_pieces[ROOK]; pieces; ClearLow(pieces))
    {
        from = LSB(pieces);
        moves = cached->moves_rook[from]
                & ~cached->directions[NORTH][LSB(cached->directions[NORTH][from] & state->occupied_both)]
                & ~cached->directions[EAST][LSB(cached->directions[EAST][from] & state->occupied_both)]
                & ~cached->directions[SOUTH][MSB(cached->directions[SOUTH][from] & state->occupied_both)]
                & ~cached->directions[WEST][MSB(cached->directions[WEST][from] & state->occupied_both)]
                & not_occupied;

        for (; moves; ClearLow(moves))
        {
            to = LSB(moves);
            *movebuf++ = PackMove(from, to, ROOK, state->square[to], -1);
            (*count)++;
        }
    }

    /* QUEEN */
    for (pieces = my_pieces[QUEEN]; pieces; ClearLow(pieces))
    {
        from = LSB(pieces);
        moves = (cached->moves_rook[from] | cached->moves_bishop[from])
                & ~cached->directions[NW][LSB(cached->directions[NW][from] & state->occupied_both)]
                & ~cached->directions[NE][LSB(cached->directions[NE][from] & state->occupied_both)]
                & ~cached->directions[SE][MSB(cached->directions[SE][from] & state->occupied_both)]
                & ~cached->directions[SW][MSB(cached->directions[SW][from] & state->occupied_both)]
                & ~cached->directions[NORTH][LSB(cached->directions[NORTH][from] & state->occupied_both)]
                & ~cached->directions[EAST][LSB(cached->directions[EAST][from] & state->occupied_both)]
                & ~cached->directions[SOUTH][MSB(cached->directions[SOUTH][from] & state->occupied_both)]
                & ~cached->directions[WEST][MSB(cached->directions[WEST][from] & state->occupied_both)]
                & not_occupied;

        for (; moves; ClearLow(moves))
        {
            to = LSB(moves);
            *movebuf++ = PackMove(from, to, QUEEN, state->square[to], -1);
            (*count)++;
        }
    }

    /* PAWN */
    for (pieces = my_pieces[PAWN]; pieces; ClearLow(pieces))
    {
        from = LSB(pieces);
        moves = cached->moves_pawn_one[state->turn][from] & ~state->occupied_both;
        if (moves)
        {
            moves = cached->moves_pawn_two[state->turn][from] & ~state->occupied_both;
        }
        moves |= cached->attacks_pawn[state->turn][from] & state->occupied[1 - state->turn];

        for (; moves; ClearLow(moves))
        {
            to = LSB(moves);

            if ((state->turn == WHITE && to >= 56) || (state->turn == BLACK && to < 8))
            {
                *movebuf++ = PackMove(from, to, PAWN, state->square[to], QUEEN);
                (*count)++;
                *movebuf++ = PackMove(from, to, PAWN, state->square[to], ROOK);
                (*count)++;
                *movebuf++ = PackMove(from, to, PAWN, state->square[to], BISHOP);
                (*count)++;
                *movebuf++ = PackMove(from, to, PAWN, state->square[to], KNIGHT);
                (*count)++;
            }
            else
            {
                *movebuf++ = PackMove(from, to, PAWN, state->square[to], -1);
                (*count)++;
            }
        }

        if (cached->attacks_pawn[state->turn][from] & state->en_passant)
        {
            to = LSB(state->en_passant);
            *movebuf++ = PackMove(from, to, PAWN, PAWN, -1);
            (*count)++;
        }
    }

    /* KING */
    from = state->king_idx[state->turn];
    moves = cached->moves_king[from] & not_occupied;

    for (; moves; ClearLow(moves))
    {
        to = LSB(moves);
        *movebuf++ = PackMove(from, to, KING, state->square[to], -1);
        (*count)++;
    }
}

int move_is_attacked(state_t *state, int square_idx, int attacker)
{
    /* Checks if a set of squares are currently attacked by an attackers pieces */

    if (state->pieces[attacker][PAWN] & cached->attacked_by_pawn[attacker][square_idx])
        return 1;

    if (state->pieces[attacker][KNIGHT] & cached->moves_knight[square_idx])
        return 1;

    if (state->pieces[attacker][KING] & cached->moves_king[square_idx])
        return 1;

    uint64_t bishop_and_queen = state->pieces[attacker][BISHOP] | state->pieces[attacker][QUEEN];

    uint64_t nw_hits = state->occupied_both & cached->directions[NW][square_idx];
    if ((nw_hits & -nw_hits) & bishop_and_queen)
        return 1;

    uint64_t ne_hits = state->occupied_both & cached->directions[NE][square_idx];
    if ((ne_hits & -ne_hits) & bishop_and_queen)
        return 1;

    uint64_t sw_hits = state->occupied_both & cached->directions[SW][square_idx];
    if (sw_hits && ((1ull << MSB(sw_hits)) & bishop_and_queen))
        return 1;

    uint64_t se_hits = state->occupied_both & cached->directions[SE][square_idx];
    if (se_hits && ((1ull << MSB(se_hits)) & bishop_and_queen))
        return 1;

    uint64_t rook_and_queen = state->pieces[attacker][ROOK] | state->pieces[attacker][QUEEN];

    uint64_t north_hits = state->occupied_both & cached->directions[NORTH][square_idx];
    if ((north_hits & -north_hits) & rook_and_queen)
        return 1;

    uint64_t east_hits = state->occupied_both & cached->directions[EAST][square_idx];
    if ((east_hits & -east_hits) & rook_and_queen)
        return 1;

    uint64_t south_hits = state->occupied_both & cached->directions[SOUTH][square_idx];
    if (south_hits && ((1ull << MSB(south_hits)) & rook_and_queen))
        return 1;

    uint64_t west_hits = state->occupied_both & cached->directions[WEST][square_idx];
    if (west_hits && ((1ull << MSB(west_hits)) & rook_and_queen))
        return 1;

    return 0;
}

void move_make(state_t *state, int move, int ply)
{
    register uint64_t from_square, to_square;
    register int from, to, piece, capture, promote;
    register int opponent;

    /* Save some data for unmake_move */
    state->old_castling[ply] = state->castling;
    state->old_en_passant[ply] = state->en_passant;
    state->old_zobrist[ply] = state->zobrist;

    from = MoveFrom(move);
    to = MoveTo(move);
    piece = MovePiece(move);
    capture = MoveCapture(move);
    promote = MovePromote(move);

    from_square = 1ull << from;
    to_square = 1ull << to;
    opponent = 1 - state->turn;

    /* Remove the piece that moved from the board. */
    state->pieces[state->turn][piece] &= ~from_square;
    state->occupied[state->turn] &= ~from_square;
    state->zobrist ^= hash_zobrist->pieces[state->turn][piece][from];
    state->square[from] = -1;

    /* Update the board with the new position of the piece. */
    if (promote < 6)
    {
        state->pieces[state->turn][promote] ^= to_square;
        state->zobrist ^= hash_zobrist->pieces[state->turn][promote][to];
        state->square[to] = promote;
    }
    else
    {
        state->pieces[state->turn][piece] ^= to_square;
        state->zobrist ^= hash_zobrist->pieces[state->turn][piece][to];
        state->square[to] = piece;
    }

    /* If it is a capture, we need to remove the opponent piece as well. */
    if (capture < 6)
    {
        /* Remember to clear castling availability if we capture a rook. */
        if (state->castling & to_square)
        {
            state->castling &= ~to_square;
            state->zobrist ^= hash_zobrist->castling[to];
        }

        uint64_t to_remove_square = to_square;
        int to_remove_square_idx = to;

        if (piece == PAWN && to_square & state->en_passant)
        {
            /* The piece captured with en passant; we need to clear the board of the captured piece.
                * We simply use the pawn move square of the opponent to find out which square to clear. */
            to_remove_square = cached->moves_pawn_one[opponent][to];
            to_remove_square_idx = LSB(to_remove_square);

            state->square[to_remove_square_idx] = -1;
        }

        /* Remove the captured piece off the board. */
        state->pieces[opponent][capture] ^= to_remove_square;
        state->occupied[opponent] ^= to_remove_square;
        state->zobrist ^= hash_zobrist->pieces[opponent][capture][to_remove_square_idx];
    }

    /* Update "occupied" with the same piece as above. */
    state->occupied[state->turn] ^= to_square;
    
    if (state->en_passant)
    {
        state->zobrist ^= hash_zobrist->en_passant[LSB(state->en_passant)];
        state->en_passant = 0;
    }

    uint64_t rook_mask;
    switch (piece)
    {
        case PAWN:
            /* set en_passant? */
            if (~cached->moves_pawn_one[state->turn][from] & to_square & cached->moves_pawn_two[state->turn][from])
            {
                state->en_passant = cached->moves_pawn_one[state->turn][from];
                state->zobrist ^= hash_zobrist->en_passant[LSB(state->en_passant)];
            }
            break;

        case KNIGHT:
        case BISHOP:
            break;

        case ROOK:
            /* Clear the appropriate castling availability. */
            if (state->castling & from_square)
            {
                state->castling &= ~from_square;
                state->zobrist ^= hash_zobrist->castling[from];
            }
            break;

        case QUEEN:
            break;

        case KING:
            rook_mask = cached->castling_rookmask[state->turn][from][to];

            state->pieces[state->turn][ROOK] ^= rook_mask;
            state->occupied[state->turn] ^= rook_mask;
            state->zobrist ^= hash_zobrist->rook_castling[state->turn][from][to];

            state->king_idx[state->turn] = to;

            for (;rook_mask; ClearLow(rook_mask))
            {
                int tmp = LSB(rook_mask);
                state->square[tmp] = (state->square[tmp] == -1 ? ROOK : -1);
            }

            #if 0
            /* These three lines are apparently slower than the branching/LSB thingie below ..*/
            uint64_t castling = state->castling & cached->castling_by_color[state->turn];
            state->castling ^= castling;
            state->zobrist ^= hash_zobrist->state_castling[state->turn][castling >> (56 * state->turn)];
            #endif

            uint64_t castling = state->castling & cached->castling_by_color[state->turn];
            state->castling ^= castling;

            while (castling)
            {
                int castling_idx = LSB(castling);
                castling &= castling - 1;
                state->zobrist ^= hash_zobrist->castling[castling_idx];
            }
            break;
    }
    
    state->turn ^= 1;
    state->zobrist ^= hash_zobrist->turn;

    state->occupied_both = state->occupied[WHITE] | state->occupied[BLACK];
}

void move_unmake(state_t *state, int move, int ply)
{
    register uint64_t from_square, to_square;
    register int from, to, piece, capture, promote;
    register int opponent;

    /* Save some data for unmake_move */
    state->castling = state->old_castling[ply];
    state->en_passant = state->old_en_passant[ply];
    state->zobrist = state->old_zobrist[ply];

    from = MoveFrom(move);
    to = MoveTo(move);
    piece = MovePiece(move);
    capture = MoveCapture(move);
    promote = MovePromote(move);

    from_square = 1ull << from;
    to_square = 1ull << to;
    opponent = state->turn;
    state->turn ^= 1;

    /* Update from/to squares */
    state->pieces[state->turn][piece] |= from_square;
    state->occupied[state->turn] |= from_square;
    state->square[from] = piece;

    state->occupied[state->turn] ^= to_square;
    state->square[to] = -1;

    if (promote < 6)
    {
        state->pieces[state->turn][promote] ^= to_square;
    }
    else
    {
        state->pieces[state->turn][piece] ^= to_square;
    }

    /* If it is a capture, we need to remove the opponent piece as well. */
    if (capture < 6)
    {
        uint64_t to_remove_square = to_square;
        if (piece == PAWN && to_square & state->en_passant)
        {
            /* The piece captured with en passant; we need to clear the board of the captured piece.
                * We simply use the pawn move square of the opponent to find out which square to clear. */
            to_remove_square = cached->moves_pawn_one[opponent][to];
        }

        /* Remove the captured piece off the board. */
        state->pieces[opponent][capture] ^= to_remove_square;
        state->occupied[opponent] ^= to_remove_square;
        state->square[LSB(to_remove_square)] = capture;
    }

    uint64_t rook_mask;
    switch (piece)
    {
        case PAWN:
        case KNIGHT:
        case BISHOP:
        case QUEEN:
            break;

        case KING:
            rook_mask = cached->castling_rookmask[state->turn][from][to];
            state->pieces[state->turn][ROOK] ^= rook_mask;
            state->occupied[state->turn] ^= rook_mask;

            for (;rook_mask; ClearLow(rook_mask))
            {
                int tmp = LSB(rook_mask);
                state->square[tmp] = (state->square[tmp] == -1 ? ROOK : -1);
            }

            state->king_idx[state->turn] = from;
            break;
    }
    
    state->occupied_both = state->occupied[WHITE] | state->occupied[BLACK];
}
