#define _GNU_SOURCE
#include <stdlib.h>
#include <string.h>
#include "move.h"
#include "util.h"
#include "defines.h"
#include "cache.h"

/* File containing functions for generating moves */

void move_to_string(move_t *move, char *out)
{
    int from_square = ffsll(move->from_square) - 1,
        to_square = ffsll(move->to_square) - 1,
        i = 0;
    char *buf = malloc(sizeof(char) * 2);

    util_square_to_chars(from_square, buf);

    out[i++] = buf[0];
    out[i++] = buf[1];

    out[i++] = (move->capture >= 0) ? 'x' : ' ';

    util_square_to_chars(to_square, buf);

    out[i++] = buf[0];
    out[i++] = buf[1];

    free(buf);
}

void move_generate_moves(state_t *state, move_t *moves, int *count)
{
    /* Returns a list of the available moves/captures/promotions in a position, for the player in turn.

       The generated moves might leave the king in check (invalid move), so this has to be checked elsewhere. */
    int opponent = 1 - state->turn,
        piece;

    for (piece = 0; piece < 6; ++piece)
    {
        uint64_t bits = state->pieces[state->turn][piece];

        while (bits)
        {
            uint64_t from_square = bits & -bits;
            bits &= bits - 1;

            uint64_t valid_moves = move_piece_moves(state, state->turn, piece, from_square);
            while (valid_moves)
            {
                uint64_t to_square = valid_moves & -valid_moves;
                valid_moves &= valid_moves - 1;
                
                /* Check if it's a capture. If so, set "capture" to the captured piece. */
                int capture = -1;
                if (to_square & state->occupied_both)
                {
                    for (capture = 0; capture < 6; ++capture)
                    {
                        if (to_square & state->pieces[opponent][capture])
                        {
                            break;
                        }
                    }

                    if (capture == KING)
                    {
                        /* This means that the position is invalid, since the king can be captured */
                        *count = -1;
                        return;
                    }
                }

                /* En passant is a capture as well. */
                /* TODO: else if */
                if (piece == PAWN && to_square & state->en_passant)
                {
                    capture = PAWN;
                }

                /* Check if it's a promotion. If so, generate a move for all possible promotions. */
                if (piece == PAWN && to_square & cached->promotion_rank[state->turn])
                {
                    int promotion;
                    for (promotion = KNIGHT; promotion < KING; ++promotion)
                    {
                        moves[*count].from_square = from_square;
                        moves[*count].to_square = to_square;
                        moves[*count].from_piece = piece;
                        moves[*count].capture = capture;
                        moves[*count].promotion = promotion;

                        ++(*count);
                    }
                }
                else
                {
                    /* If it's not a promotion, we'll just generate one move. */
                    moves[*count].from_square = from_square;
                    moves[*count].to_square = to_square;
                    moves[*count].from_piece = piece;
                    moves[*count].capture = capture;
                    moves[*count].promotion = -1;

                    ++(*count);
                }
            }
        }
    }
}

uint64_t move_piece_moves(state_t *state, int color, int piece, uint64_t from_square)
{
    /* Returns a 64bit int containing the valid moves/captures of one specific piece in a position
     * The generated moves might leave the king in check (invalid move), so this has to be checked elsewhere. */
    uint64_t valid_moves = 0;
    int opponent = 1 - color,
        from_idx = ffsll(from_square) - 1;

    if (piece == PAWN)
    {
        /* First, we check if a one-step move is available, and if so,
         * we set valid_moves to two steps forwards (since we know
         * that the first step wasn't blocked by a piece). */
        valid_moves = cached->moves_pawn_one[color][from_idx] & ~state->occupied_both;

        if (valid_moves)
        {
            valid_moves = cached->moves_pawn_two[color][from_idx] & ~state->occupied_both;
        }

        valid_moves |= cached->attacks_pawn[color][from_idx] & state->en_passant;

        /* Check the attack-pattern against opponents and/or en passant availablility. */
        valid_moves |= cached->attacks_pawn[color][from_idx] & state->occupied[opponent];

    }
    else if (piece == KNIGHT)
    {
        valid_moves = cached->moves_knight[from_idx] & ~state->occupied[color];

    }
    else if (piece == KING)
    {
        valid_moves = cached->moves_king[from_idx] & ~state->occupied[color];

        /* We need to first check if the path is free and that castling is available in that direction.
         * Then we need to see if the king or any of the "stepping" squares
         * (F1 and G1 for white king side castle, for instance) are being attacked. */

        uint64_t left_castle = cached->castling_availability[color][0][from_idx];
        if (left_castle & state->castling)
        {
            uint64_t steps = cached->castling_steps[color][0];
            uint64_t move_steps = steps | left_castle << 1;
            if ((0 == (move_steps & state->occupied_both)) && !move_is_attacked(state, steps | from_square, opponent))
            {
                valid_moves |= left_castle << 2;
            }
        }

        uint64_t right_castle = cached->castling_availability[color][1][from_idx];
        if (right_castle & state->castling)
        {
            uint64_t steps = cached->castling_steps[color][1];
            if ((0 == (steps & state->occupied_both)) && !move_is_attacked(state, steps | from_square, opponent))
            {
                valid_moves |= right_castle >> 1;
            }
        }
    }
    else
    {
        /* TODO: Needs more rotated or magic or something bitboards! */
        if (piece == BISHOP || piece == QUEEN)
        {
            uint64_t nw_moves = cached->directions[NW][from_idx] & state->occupied_both;
            nw_moves = (nw_moves << 7) | (nw_moves << 14)
                        | (nw_moves << 21) | (nw_moves << 28)
                        | (nw_moves << 35) | (nw_moves << 42);
            nw_moves &= cached->directions[NW][from_idx];
            nw_moves ^= cached->directions[NW][from_idx];

            uint64_t ne_moves = cached->directions[NE][from_idx] & state->occupied_both;
            ne_moves = (ne_moves << 9) | (ne_moves << 18)
                        | (ne_moves << 27) | (ne_moves << 36)
                        | (ne_moves << 45) | (ne_moves << 54);
            ne_moves &= cached->directions[NE][from_idx];
            ne_moves ^= cached->directions[NE][from_idx];

            uint64_t se_moves = cached->directions[SE][from_idx] & state->occupied_both;
            se_moves = (se_moves >> 7) | (se_moves >> 14)
                        | (se_moves >> 21) | (se_moves >> 28)
                        | (se_moves >> 35) | (se_moves >> 42);
            se_moves &= cached->directions[SE][from_idx];
            se_moves ^= cached->directions[SE][from_idx];

            uint64_t sw_moves = cached->directions[SW][from_idx] & state->occupied_both;
            sw_moves = (sw_moves >> 9) | (sw_moves >> 18)
                        | (sw_moves >> 27) | (sw_moves >> 36)
                        | (sw_moves >> 45) | (sw_moves >> 54);
            sw_moves &= cached->directions[SW][from_idx];
            sw_moves ^= cached->directions[SW][from_idx];

            valid_moves |= (nw_moves | ne_moves | se_moves | sw_moves) & ~state->occupied[color];
        }

        if (piece == ROOK || piece == QUEEN)
        {
            uint64_t right_moves = cached->directions[EAST][from_idx] & state->occupied_both;
            right_moves = (right_moves << 1) | (right_moves << 2)
                        | (right_moves << 3) | (right_moves << 4)
                        | (right_moves << 5) | (right_moves << 6);
            right_moves &= cached->directions[EAST][from_idx];
            right_moves ^= cached->directions[EAST][from_idx];

            uint64_t left_moves = cached->directions[WEST][from_idx] & state->occupied_both;
            left_moves = (left_moves >> 1) | (left_moves >> 2)
                        | (left_moves >> 3) | (left_moves >> 4)
                        | (left_moves >> 5) | (left_moves >> 6);
            left_moves &= cached->directions[WEST][from_idx];
            left_moves ^= cached->directions[WEST][from_idx];

            uint64_t up_moves = cached->directions[NORTH][from_idx] & state->occupied_both;
            up_moves = (up_moves << 8) | (up_moves << 16)
                        | (up_moves << 24) | (up_moves << 32)
                        | (up_moves << 40) | (up_moves << 48);
            up_moves &= cached->directions[NORTH][from_idx];
            up_moves ^= cached->directions[NORTH][from_idx];

            uint64_t down_moves = cached->directions[SOUTH][from_idx] & state->occupied_both;
            down_moves = (down_moves >> 8) | (down_moves >> 16)
                        | (down_moves >> 24) | (down_moves >> 32)
                        | (down_moves >> 40) | (down_moves >> 48);
            down_moves &= cached->directions[SOUTH][from_idx];
            down_moves ^= cached->directions[SOUTH][from_idx];

            valid_moves |= (right_moves | left_moves | up_moves | down_moves) & ~state->occupied[color];
        }
    }

    return valid_moves;
}


int move_is_attacked(state_t *state, uint64_t squares, int attacker)
{
    /* Checks if a set of squares are currently attacked by an attackers pieces */
    int defender = 1 - attacker;

    while (squares)
    {
        uint64_t square = squares & -squares;
        squares &= squares - 1;

        int square_idx = ffsll(square) - 1;

        /* Checking whether a pawn, knight or a king is attacking a square by using
         * bitwise and on the squares they can possibly attack from. */
        if (state->pieces[attacker][PAWN] & cached->attacked_by_pawn[attacker][square_idx])
        {
            return 1;
        }
        else if (state->pieces[attacker][KNIGHT] & cached->moves_knight[square_idx])
        {
            return 1;
        }
        else if (state->pieces[attacker][KING] & cached->moves_king[square_idx])
        {
            return 1;
        }
        /* TODO:
         * This can be much faster if we check every direction (4 for bishop, 4 for rooks),
         * and see if the direction from that square hits bishop_and_queen / rook_and_queen.
         * We need to alternate between using the highest and the lowest bit to bitwise and
         * both rook and bishop moves.
         * http://www.talkchess.com/forum/viewtopic.php?t=30356 */

        /* Pretend to generate moves from the defenders POV, and see if the valid moves fits with
         * a black bishop, rook or queen on the board. */
        else if ((state->pieces[attacker][BISHOP] | state->pieces[attacker][QUEEN]) & move_piece_moves(state, defender, BISHOP, square))
        {
            return 1;
        }
        else if ((state->pieces[attacker][ROOK] | state->pieces[attacker][QUEEN]) & move_piece_moves(state, defender, ROOK, square))
        {
            return 1;
        }
    }

    return 0;
}

void move_make_move(state_t* state, move_t* move)
{
    int to_square_idx = ffsll(move->to_square) - 1,
        from_square_idx = ffsll(move->from_square) - 1;

    /* Save some data for unmake_move */
    move->castling = state->castling;
    move->en_passant = state->en_passant;

    int opponent = 1 - state->turn;

    /* Remove the piece that moved from the board. */
    state->pieces[state->turn][move->from_piece] ^= move->from_square;
    state->occupied[state->turn] ^= move->from_square;

    /* If it is a capture, we need to remove the opponent piece as well. */
    if (move->capture >= 0)
    {
        /* Remember to clear castling availability if we capture a rook. */
        if (state->castling & move->to_square)
        {
            state->castling &= ~move->to_square;
        }

        uint64_t to_remove_square = move->to_square;
        if (move->from_piece == PAWN && move->to_square & state->en_passant)
        {
            /* The piece captured with en passant; we need to clear the board of the captured piece.
                * We simply use the pawn move square of the opponent to find out which square to clear. */
            to_remove_square = cached->moves_pawn_one[opponent][to_square_idx];
        }

        /* Remove the captured piece off the board. */
        state->pieces[opponent][move->capture] ^= to_remove_square;
        state->occupied[opponent] ^= to_remove_square;
    }

    /* Update the board with the new position of the piece. */
    if (move->promotion >= 0)
    {
        state->pieces[state->turn][move->promotion] ^= move->to_square;
    }
    else
    {
        state->pieces[state->turn][move->from_piece] ^= move->to_square;
    }

    /* Update "occupied" with the same piece as above. */
    state->occupied[state->turn] ^= move->to_square;
    
    state->en_passant = 0;

    if (move->from_piece == KING)
    {
        /* TODO: This can be made more efficient by caching more stuff..
            * We could first see if the move was >1 step (one bitwise and and one lookup),
            * then we could have a cache element where cached[to_square] gives the place where
            * the rook should be positioned (one bitwise xor and one lookup). */

        uint64_t left_castle = cached->castling_availability[state->turn][0][from_square_idx];
        if ((left_castle << 2) & move->to_square)
        {
            state->pieces[state->turn][ROOK] ^= left_castle | left_castle << 3;
            state->occupied[state->turn] ^= left_castle | left_castle << 3;
        }

        uint64_t right_castle = cached->castling_availability[state->turn][1][from_square_idx];
        if ((right_castle >> 1) & move->to_square)
        {
            state->pieces[state->turn][ROOK] ^= right_castle | right_castle >> 2;
            state->occupied[state->turn] ^= right_castle | right_castle >> 2;
        }

        /* Clear the appropriate castling availability. */
        state->castling &= ~cached->castling_by_color[state->turn];

    }
    else if (move->from_piece == ROOK)
    {
        /* Clear the appropriate castling availability. */
        state->castling &= ~move->from_square;
    }
    else if (move->from_piece == PAWN)
    {
        /* Clear / set en_passant. */
        if (~cached->moves_pawn_one[state->turn][from_square_idx] & move->to_square & cached->moves_pawn_two[state->turn][from_square_idx])
        {
            state->en_passant = cached->moves_pawn_one[state->turn][from_square_idx];
        }
    }
    
    state->turn ^= 1;
    state->occupied_both = state->occupied[WHITE] | state->occupied[BLACK];
}
