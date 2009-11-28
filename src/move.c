#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "move.h"
#include "util.h"
#include "cache.h"
#include "hash.h"
#include "plp.h"

void move_to_string(move_t *move, char *out)
{
    /* Makes a string out of a move_t struct */
    int i = 0;

    char buf[2];

    util_square_to_chars(MoveFrom(move), buf);

    out[i++] = buf[0];
    out[i++] = buf[1];

    util_square_to_chars(MoveTo(move), buf);

    out[i++] = buf[0];
    out[i++] = buf[1];

    if (MovePromote(move) >= 0)
    {
        out[i++] = util_piece_to_char(BLACK, MovePromote(move));
    }

    out[i++] = '\0';
}

void move_generate_moves(state_t *state, move_t *moves, int *count)
{
    /* Returns a list of the available moves/captures/promotions in a position, for the player in turn.
       The generated moves might leave the king in check (invalid move), so this has to be checked elsewhere.
       In this scenario, *count will be set to -1. */
    int opponent = 1 - state->turn;
    int piece;
    for (piece = PAWN; piece <= KING; ++piece)
    {
        uint64_t bits = state->pieces[state->turn][piece];

        while (bits)
        {
            uint64_t from_square = bits & -bits;
            int from = LSB(from_square);
            bits &= bits - 1;

            uint64_t valid_moves = move_piece_moves(state, state->turn, piece, from);
            while (valid_moves)
            {
                uint64_t to_square = valid_moves & -valid_moves;
                int to = LSB(to_square);
                valid_moves &= valid_moves - 1;
                
                if (to == state->king_idx[opponent])
                {
                    /* This means that the position is invalid, since the king can be captured */
                    *count = -1;
                    return;
                }

                int capture = state->square[to];

                /* En passant is a capture as well. */
                if (piece == PAWN && to_square & state->en_passant)
                {
                    capture = PAWN;
                }

                /* Check if it's a promotion. If so, generate a move for all possible promotions. */
                if (piece == PAWN && to_square & cached->promotion_rank[state->turn])
                {
                    int promotion;
                    for (promotion = QUEEN; promotion >= KNIGHT; --promotion)
                    {
                        moves[*count].from = from;
                        moves[*count].to = to;
                        moves[*count].piece = piece;
                        moves[*count].capture = capture;
                        moves[*count].promotion = promotion;

                        ++(*count);
                    }
                }
                else
                {
                    /* If it's not a promotion, we'll just generate one move. */
                    moves[*count].from = from;
                    moves[*count].to = to;
                    moves[*count].piece = piece;
                    moves[*count].capture = capture;
                    moves[*count].promotion = -1;

                    ++(*count);
                }
            }
        }
    }
}

uint64_t move_piece_moves(state_t *state, int color, int piece, int from_idx)
{
    /* Returns a 64bit int containing the valid moves/captures of one specific piece in a position
     * The generated moves might leave the king in check (invalid move), so this has to be checked elsewhere. */
    uint64_t valid_moves = 0;
    int opponent = 1 - color;

    switch (piece)
    {
        case PAWN:
            /* First, we check if a one-step move is available, and if so,
            * we set valid_moves to two steps forwards (since we know
            * that the first step wasn't blocked by a piece). */
            valid_moves = cached->moves_pawn_one[color][from_idx] & ~state->occupied_both;

            if (valid_moves)
            {
                valid_moves = cached->moves_pawn_two[color][from_idx] & ~state->occupied_both;
            }

            /* Check the attack-pattern against opponents and/or en passant availablility. */
            valid_moves |= cached->attacks_pawn[color][from_idx] & (state->en_passant | state->occupied[opponent]);
            break;

        case KNIGHT:
            valid_moves = cached->moves_knight[from_idx] & ~state->occupied[color];
            break;

        case KING:
            valid_moves = cached->moves_king[from_idx] & ~state->occupied[color];

            /* We need to first check if the path is free and that castling is available in that direction.
            * Then we need to see if the king or any of the "stepping" squares (F1 and G1 for white king side castle,
            * for instance) are being attacked. */

            {
                uint64_t left_castle = cached->castling_availability[color][0][from_idx];
                if (left_castle & state->castling)
                {
                    uint64_t steps = cached->castling_steps[color][0];
                    uint64_t move_steps = steps | left_castle << 1;
                    if ((0 == (move_steps & state->occupied_both)))
                    {
                        if (move_is_attacked(state, from_idx, opponent))
                        {
                            goto nocastle_1;
                        }
                        while (steps)
                        {
                            if (move_is_attacked(state, LSB(steps), opponent))
                            {
                                goto nocastle_1;
                            }
                            steps &= steps - 1;
                        }
                        valid_moves |= left_castle << 2;
                    }
                }
            }

            nocastle_1:

            {
                uint64_t right_castle = cached->castling_availability[color][1][from_idx];
                if (right_castle & state->castling)
                {
                    uint64_t steps = cached->castling_steps[color][1];
                    if ((0 == (steps & state->occupied_both)))
                    {
                        if (move_is_attacked(state, from_idx, opponent))
                        {
                            goto nocastle_2;
                        }
                        while (steps)
                        {
                            if (move_is_attacked(state, LSB(steps), opponent))
                            {
                                goto nocastle_2;
                            }
                            steps &= steps - 1;
                        }
                        valid_moves |= right_castle >> 1;
                    }
                }
            }

            nocastle_2:
            break;

        /* Check each direction and see if there are any blockers. If there are, take the nearest blocker
        * (might be LSB and might be MSB) and remove the path in the same direction
        * with the blocker as source this time. */
        case QUEEN:
            valid_moves = (cached->moves_rook[from_idx] | cached->moves_bishop[from_idx])
                            & ~cached->directions[NW][LSB(cached->directions[NW][from_idx] & state->occupied_both)]
                            & ~cached->directions[NE][LSB(cached->directions[NE][from_idx] & state->occupied_both)]
                            & ~cached->directions[SE][MSB(cached->directions[SE][from_idx] & state->occupied_both)]
                            & ~cached->directions[SW][MSB(cached->directions[SW][from_idx] & state->occupied_both)]
                            & ~cached->directions[NORTH][LSB(cached->directions[NORTH][from_idx] & state->occupied_both)]
                            & ~cached->directions[EAST][LSB(cached->directions[EAST][from_idx] & state->occupied_both)]
                            & ~cached->directions[SOUTH][MSB(cached->directions[SOUTH][from_idx] & state->occupied_both)]
                            & ~cached->directions[WEST][MSB(cached->directions[WEST][from_idx] & state->occupied_both)]
                            & ~state->occupied[color];
            break;
        case BISHOP:
            valid_moves = cached->moves_bishop[from_idx]
                            & ~cached->directions[NW][LSB(cached->directions[NW][from_idx] & state->occupied_both)]
                            & ~cached->directions[NE][LSB(cached->directions[NE][from_idx] & state->occupied_both)]
                            & ~cached->directions[SE][MSB(cached->directions[SE][from_idx] & state->occupied_both)]
                            & ~cached->directions[SW][MSB(cached->directions[SW][from_idx] & state->occupied_both)]
                            & ~state->occupied[color];
            break;
        case ROOK:
            valid_moves = cached->moves_rook[from_idx]
                            & ~cached->directions[NORTH][LSB(cached->directions[NORTH][from_idx] & state->occupied_both)]
                            & ~cached->directions[EAST][LSB(cached->directions[EAST][from_idx] & state->occupied_both)]
                            & ~cached->directions[SOUTH][MSB(cached->directions[SOUTH][from_idx] & state->occupied_both)]
                            & ~cached->directions[WEST][MSB(cached->directions[WEST][from_idx] & state->occupied_both)]
                            & ~state->occupied[color];
            break;
    }

    return valid_moves;
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

void move_make(state_t *state, move_t *move, int ply)
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
    if (promote >= 0)
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
    if (capture >= 0)
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

void move_unmake(state_t *state, move_t *move, int ply)
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

    if (promote >= 0)
    {
        state->pieces[state->turn][promote] ^= to_square;
    }
    else
    {
        state->pieces[state->turn][piece] ^= to_square;
    }

    /* If it is a capture, we need to remove the opponent piece as well. */
    if (capture >= 0)
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
