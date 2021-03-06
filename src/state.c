#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>
#include "state.h"
#include "eval.h"
#include "cache.h"
#include "util.h"
#include "hash.h"
#include "plp.h"

int state_init_from_fen(state_t *state, char *fen)
{
    /* Sets the board according to Forsyth-Edwards Notation.
     * See http://en.wikipedia.org/wiki/Forsyth%E2%80%93Edwards_Notation for more information.
     *
     * Returns the number of used characters from fen */

    int piece_idx = 8 * 7; /* Starting on the A8. */
    int i = 0;
    int color, piece;
    char num[16];
    int res;

    memset(state, 0, sizeof(state_t));
    memset(state->square, 7, 64 * sizeof(int));

    /* Set up the position. */
    while (fen[i] && fen[i] != ' ')
    {
        char c = fen[i];

        if (c == '/')
        {
            piece_idx -= 2 * 8;
        }
        else if (isdigit(c))
        {
            piece_idx += (int)(c - '0');
        }
        else
        {
            int piece = util_char_to_piece(c);
            int color = util_char_to_color(c);
            state->pieces[color][piece] |= (1ull << piece_idx);
            state->square[piece_idx] = piece;
            if (piece == KING)
            {
                state->king_idx[color] = piece_idx;
            }

            piece_idx += 1;
        }

        ++i;
    }

    ++i;

    /* Update state->occupied with the occupied squares in state->pieces. */
    for (color = 0; color < 2; ++color)
    {
        for (piece = 0; piece < 6; ++piece)
        {
            state->occupied[color] |= state->pieces[color][piece];
            state->occupied_both |= state->pieces[color][piece];
        }
    }

    /* Set active color. */
    if (tolower(fen[i++]) == 'w')
    {
        state->turn = WHITE;
    }
    else
    {
        state->turn = BLACK;
    }

    ++i;

    while (fen[i] && fen[i] != ' ')
    {
        switch (fen[i++])
        {
            case 'Q':
                state->castling |= A1;
                break;
            case 'K':
                state->castling |= H1;
                break;
            case 'q':
                state->castling |= A8;
                break;
            case 'k':
                state->castling |= H8;
                break;
        }
    }

    ++i;

    /* Set en passant. */
    if (fen[i] != '-')
    {
        uint64_t square_idx = util_chars_to_square(&fen[i]);
        state->en_passant = (1ull << square_idx);

        i += 2;
    }
    else
    {
        ++i;
    }

    ++i;

    /* TODO: Halfmove and Fullmove numbers from FEN. */
    /* Halfmove */

    res = sscanf(&fen[i], "%s", num);
    if (1 == res && num[0] != '-')
    {
        //int halfmove = atoi(num);
        i += strlen(num);
    }
    else
    {
        ++i;
    }

    ++i;

    /* Fullmove */
    res = sscanf(&fen[i], "%s", num);
    if (1 == res && num[0] != '-')
    {
        //int fullmove = atoi(num);
        i += strlen(num);
    }
    else
    {
        ++i;
    }

    state->zobrist = hash_make_zobrist(state);
    return i;
}

int state_is_repeating(state_t *state)
{
    int i;

    if (state->moves < 10)
    {
        return 0;
    }

    for (i = state->moves - 3; i >= state->last_pawn_or_cap; i -= 2)
    {
        if (state->repetition[i] == state->zobrist)
        {
            return 1;
        }
    }

    return 0;
}

void state_print(state_t *state)
{
    /*Makes a pretty string, representing a position.*/

    char *seperator = "+---+---+---+---+---+---+---+---+\n";
    int y, x;

    printf("%s", seperator);

    for (y = 7; y >= 0; --y)
    {
        printf("|");

        for (x = 0; x < 8; ++x)
        {
            uint64_t idx = 1ull << (y * 8 + x);

            char found = '\0';
            int color, piece;
            for (color = 0; color < 2; ++color)
            {
                for (piece = 0; piece < 6; ++piece)
                {
                    if (state->pieces[color][piece] & idx)
                    {
                        found = util_piece_to_char(color, piece);
                        break;
                    }
                }
            }

            char extra = ' ';
            if (state->en_passant & idx || state->castling & idx)
            {
                extra = '*';
            }

            printf(" ");

            if (found != '\0')
            {
                printf("%c", found);
            }
            else
            {
                char squareColor = (y ^ x) & 1 ? ' ' : '.';
                printf("%c", squareColor);
            }

            printf("%c|", extra);
        }

        printf("\n%s", seperator);
    }

    printf("Turn: %s\n", (state->turn == WHITE) ? "White" : "Black");
}

int state_see(state_t *state, int move)
{
    uint64_t attackers = util_attacks_to(state, MoveFrom(move)) & ~(1ull << MoveFrom(move));
    int attacked = eval_piece_values[MovePiece(move)];
    int color = Flip(state->turn);
    int ply = 1;
    int attacker;
    int swapstack[64];

    swapstack[0] = eval_real_pvalues[MoveCapture(move)];

    while (attackers)
    {
        uint64_t hits;
        for (attacker = PAWN; attacker <= KING; ++attacker)
        {
            hits = state->pieces[color][attacker] & attackers;
            if (hits)
            {
                break;
            }
        }

        if (attacker > KING)
            break;

        attackers ^= hits & -hits;
        swapstack[ply] = -swapstack[ply - 1] + attacked;
        attacked = eval_real_pvalues[attacker];
        color = Flip(color);
        ply += 1;
    }

    while (--ply)
        swapstack[ply - 1] = -Max(-swapstack[ply - 1], swapstack[ply]);

    return swapstack[0];
}
