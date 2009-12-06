#include <assert.h>
#include <string.h>
#include <ctype.h>
#include <stdio.h>
#include "util.h"
#include "cache.h"
#include "plp.h"
#include "search.h"

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

int util_chars_to_move(char *move_str, state_t *state)
{
    int from, to, piece, capture, promote;

    from = util_chars_to_square(move_str);
    move_str += 2;
    to = util_chars_to_square(move_str);
    move_str += 2;

    promote = -1;
    if (move_str[0])
    {
        promote = util_char_to_piece(move_str[0]);
    }

    capture = state->square[to];
    piece = state->square[from];

    if (piece == PAWN && (1ull << to) & state->en_passant)
    {
        capture = PAWN;
    }

    return PackMove(from, to, piece, capture, promote);
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

void util_move_to_lan(int move, char *out)
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

    if (MovePromote(move) < 6)
    {
        out[i++] = util_piece_to_char(BLACK, MovePromote(move));
    }

    out[i++] = '\0';
}

int util_legal_killer(state_t *state, int move)
{
    int from = MoveFrom(move), to = MoveTo(move);
    int piece = MovePiece(move);

    /* Killers can't be captures. This also checks for pawn bumping into a piece. */
    if (state->square[to] != -1)
        return 0;

    /* Is the piece still there? */
    if (!(state->pieces[state->turn][piece] & (1ull << from)))
        return 0;

    switch (piece)
    {
        case PAWN:
            /* Since we already knows the pawn moved to an empty square,
               we don't need to check for the destination square. */
            return !(cached->moves_pawn_one[state->turn][from] & state->occupied_both);

        case KNIGHT:
            return  1;

        case BISHOP:
            return !((1ull << to) &
                     (cached->directions[NW][LSB(cached->directions[NW][from] & state->occupied_both)]
                    | cached->directions[NE][LSB(cached->directions[NE][from] & state->occupied_both)]
                    | cached->directions[SE][MSB(cached->directions[SE][from] & state->occupied_both)]
                    | cached->directions[SW][MSB(cached->directions[SW][from] & state->occupied_both)]));

        case ROOK:
            return !((1ull << to) &
                     (cached->directions[NORTH][LSB(cached->directions[NORTH][from] & state->occupied_both)]
                    | cached->directions[EAST][LSB(cached->directions[EAST][from] & state->occupied_both)]
                    | cached->directions[SOUTH][MSB(cached->directions[SOUTH][from] & state->occupied_both)]
                    | cached->directions[WEST][MSB(cached->directions[WEST][from] & state->occupied_both)]));

        case QUEEN:
            return !((1ull << to) &
                     (cached->directions[NW][LSB(cached->directions[NW][from] & state->occupied_both)]
                    | cached->directions[NE][LSB(cached->directions[NE][from] & state->occupied_both)]
                    | cached->directions[SE][MSB(cached->directions[SE][from] & state->occupied_both)]
                    | cached->directions[SW][MSB(cached->directions[SW][from] & state->occupied_both)]
                    | cached->directions[NORTH][LSB(cached->directions[NORTH][from] & state->occupied_both)]
                    | cached->directions[EAST][LSB(cached->directions[EAST][from] & state->occupied_both)]
                    | cached->directions[SOUTH][MSB(cached->directions[SOUTH][from] & state->occupied_both)]
                    | cached->directions[WEST][MSB(cached->directions[WEST][from] & state->occupied_both)]));

        case KING:
            return Abs(from - to) != 2;
    }

    return 0;
}

void util_print_pv()
{
    int i;

    if (Abs(search.best_score) >= INF - MAX_DEPTH)
    {
        int mate_in = (INF - Abs(search.best_score)) / 2;
        printf(" score mate %d pv ", mate_in);
    }
    else
    {
        printf(" score cp %d pv ", search.best_score);
    }

    for (i = 0; i < search.pv.count; ++i)
    {
        char buf[16];
        util_move_to_lan(search.pv.moves[i], buf);
        printf("%s ", buf);
    }

    fflush(stdout);
}
