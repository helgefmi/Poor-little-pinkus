#include <string.h>
#include <ctype.h>
#include <stdio.h>
#include "util.h"
#include "defines.h"

/* TODO: Move these to cache.h */
static int piece_to_int[256];
static char int_to_piece[2][6];

void util_init()
{
    memset(piece_to_int, 1, 256 * sizeof(int));

    piece_to_int[(int)'p'] = PAWN;      piece_to_int[(int)'P'] = PAWN;
    piece_to_int[(int)'n'] = KNIGHT;    piece_to_int[(int)'N'] = KNIGHT;
    piece_to_int[(int)'b'] = BISHOP;    piece_to_int[(int)'B'] = BISHOP;
    piece_to_int[(int)'r'] = ROOK;      piece_to_int[(int)'R'] = ROOK;
    piece_to_int[(int)'q'] = QUEEN;     piece_to_int[(int)'Q'] = QUEEN;
    piece_to_int[(int)'k'] = KING;      piece_to_int[(int)'K'] = KING;

    int_to_piece[WHITE][PAWN] = 'P';    int_to_piece[BLACK][PAWN] = 'p';
    int_to_piece[WHITE][KNIGHT] = 'N';  int_to_piece[BLACK][KNIGHT] = 'n';
    int_to_piece[WHITE][BISHOP] = 'B';  int_to_piece[BLACK][BISHOP] = 'b';
    int_to_piece[WHITE][ROOK] = 'R';    int_to_piece[BLACK][ROOK] = 'r';
    int_to_piece[WHITE][QUEEN] = 'Q';   int_to_piece[BLACK][QUEEN] = 'q';
    int_to_piece[WHITE][KING] = 'K';    int_to_piece[BLACK][KING] = 'k';
}

int util_char_to_piece(char piece_c)
{
    /* Converts a piece in character-representation to its numerical piece-value. */
    return piece_to_int[(int)piece_c];
}

char util_piece_to_char(int color, int piece)
{
    return int_to_piece[color][piece];
}

int util_char_to_color(char piece_c)
{
    /* Converts a piece in character-representation to its numerical color-value. */
    return isupper(piece_c) ? WHITE : BLACK;
}


void util_square_to_chars(uint64_t square, char *out)
{
    /* Converts a numerical square value to its chess representation (0 == a1, 1 = b1, etc) */
    int y = square / 8 + 1,
        x = square % 8;
    out[0] = 'a' + (char)x;
    out[1] = '0' + (char)y;
}

uint64_t util_chars_to_square(char *move_str)
{
    /* Converts a character representation of a move (A2) to its index (8). */
    uint64_t x = (int)(tolower(move_str[0]) - 'a'),
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
