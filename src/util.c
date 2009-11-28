#include <assert.h>
#include <string.h>
#include <ctype.h>
#include <stdio.h>
#include "util.h"
#include "cache.h"
#include "plp.h"

int util_char_to_piece(char piece_c)
{
    /* Converts a piece in character-representation to its numerical piece-value. */
    return cached->piece_to_int[(int)piece_c];
}

char util_piece_to_char(int color, int piece)
{
    return cached->int_to_piece[color][piece];
}

int util_char_to_color(char piece_c)
{
    /* Converts a piece in character-representation to its numerical color-value. */
    return isupper(piece_c) ? WHITE : BLACK;
}

void util_chars_to_move(char *move_str, move_t *move, state_t *state)
{
    move->from = util_chars_to_square(move_str);
    move_str += 2;
    move->to = util_chars_to_square(move_str);
    move_str += 2;

    move->promotion = -1;
    if (move_str[0])
    {
        move->promotion = util_char_to_piece(move_str[0]);
    }

    move->capture = -1;
    move->piece = -1;

    int color, piece;
    for (color = WHITE; color <= BLACK; ++color)
    {
        for (piece = PAWN; piece <= KING; ++piece)
        {
            if (state->pieces[color][piece] & (1ull << move->from))
            {
                move->piece = piece;
            }
            if (state->pieces[color][piece] & (1ull << move->to))
            {
                move->capture = piece; 
            }
        }
    }

    assert(move->piece >= 0);

    if (move->piece == PAWN && (1ull << move->to) & state->en_passant)
    {
        move->capture = PAWN;
    }
}

void util_square_to_chars(uint64_t square, char *out)
{
    /* Converts a numerical square value to its chess representation (0 == a1, 1 = b1, etc) */
    int y = square / 8 + 1,
        x = square % 8;
    out[0] = 'a' + (char)x;
    out[1] = '0' + (char)y;
}

int util_chars_to_square(char *move_str)
{
    /* Converts a character representation of a move (A2) to its index (8). */
    int x = (int)(tolower(move_str[0]) - 'a'),
        y = (int)(move_str[1] - '0') - 1;

    return y * 8 + x;
}

void util_int_to_bitmap(uint64_t bits)
{
    /* Converts a 64bit integer to a bitmap string-representation. */
    int y, x;

    for (y = 7; y >= 0; --y)
    {
        for (x = 0; x < 8; ++x)
        {
            int idx = y * 8 + x;
            printf("%c", (bits & (1ull << idx)) ? '*' : '.');
        }
        printf("\n");
    }
    printf("\n");
}

char *util_trim_str(char *str)
{
    char *end = str + strlen(str) - 1;

    while (*str && isspace(*str))
    {
        ++str;
    }

    while (*end && isspace(*end))
    {
        --end;
    }

    *(end + 1) = '\0';
    return str;
}
