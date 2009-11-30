#include "make.h"
#include "plp.h"
#include "hash.h"
#include "cache.h"

void make_null_move(state_t *state, int ply)
{
    state->old_en_passant[ply] = state->en_passant;

    state->turn ^= 1;
    state->zobrist ^= hash_zobrist->turn;
    if (state->en_passant)
    {
        state->en_passant = 0;
        state->zobrist ^= hash_zobrist->en_passant[LSB(state->en_passant)];
    }
}

void unmake_null_move(state_t *state, int ply)
{
    state->en_passant = state->old_en_passant[ply];

    state->turn ^= 1;
    state->zobrist ^= hash_zobrist->turn;
    if (state->en_passant)
    {
        state->zobrist ^= hash_zobrist->en_passant[LSB(state->en_passant)];
    }
}

void make_move(state_t *state, int move, int ply)
{
    uint64_t from_square, to_square;
    int from, to, piece, capture, promote;
    int opponent;
    uint64_t rook_mask;

    /* Save some data for unmake_move */
    state->old_castling[ply] = state->castling;
    state->old_en_passant[ply] = state->en_passant;
    state->old_zobrist[ply] = state->zobrist;
    state->old_pawn_or_cap[ply] = state->last_pawn_or_cap;

    from = MoveFrom(move);
    to = MoveTo(move);
    piece = MovePiece(move);
    capture = MoveCapture(move);
    promote = MovePromote(move);

    from_square = 1ull << from;
    to_square = 1ull << to;
    opponent = Flip(state->turn);

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
        state->last_pawn_or_cap = state->moves;
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

    state->repetition[state->moves++] = state->zobrist;
}

void unmake_move(state_t *state, int move, int ply)
{
    uint64_t from_square, to_square;
    int from, to, piece, capture, promote;
    int opponent;
    uint64_t rook_mask;

    state->moves--;

    /* Save some data for unmake_move */
    state->castling = state->old_castling[ply];
    state->en_passant = state->old_en_passant[ply];
    state->zobrist = state->old_zobrist[ply];
    state->last_pawn_or_cap = state->old_pawn_or_cap[ply];

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

