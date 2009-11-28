#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "cache.h"
#include "plp.h"

void cache_init()
{
    /* Generates the cache from scratch.
     * Should only be called once. */
    assert(!cached);

    cached = malloc(sizeof(cached_t));
    memset(cached, 0, sizeof(cached_t));

    cached->piece_to_int[(int)'p'] = PAWN;      cached->piece_to_int[(int)'P'] = PAWN;
    cached->piece_to_int[(int)'n'] = KNIGHT;    cached->piece_to_int[(int)'N'] = KNIGHT;
    cached->piece_to_int[(int)'b'] = BISHOP;    cached->piece_to_int[(int)'B'] = BISHOP;
    cached->piece_to_int[(int)'r'] = ROOK;      cached->piece_to_int[(int)'R'] = ROOK;
    cached->piece_to_int[(int)'q'] = QUEEN;     cached->piece_to_int[(int)'Q'] = QUEEN;
    cached->piece_to_int[(int)'k'] = KING;      cached->piece_to_int[(int)'K'] = KING;

    cached->int_to_piece[WHITE][PAWN] = 'P';    cached->int_to_piece[BLACK][PAWN] = 'p';
    cached->int_to_piece[WHITE][KNIGHT] = 'N';  cached->int_to_piece[BLACK][KNIGHT] = 'n';
    cached->int_to_piece[WHITE][BISHOP] = 'B';  cached->int_to_piece[BLACK][BISHOP] = 'b';
    cached->int_to_piece[WHITE][ROOK] = 'R';    cached->int_to_piece[BLACK][ROOK] = 'r';
    cached->int_to_piece[WHITE][QUEEN] = 'Q';   cached->int_to_piece[BLACK][QUEEN] = 'q';
    cached->int_to_piece[WHITE][KING] = 'K';    cached->int_to_piece[BLACK][KING] = 'k';

    cached->castling_by_color[0] = A1 | H1;
    cached->castling_by_color[1] = A8 | H8;

    cached->castling_rooksq[WHITE][0] = A1;
    cached->castling_rooksq[WHITE][1] = H1;
    cached->castling_rooksq[BLACK][0] = A8;
    cached->castling_rooksq[BLACK][1] = H8;

    cached->castling_rookmask[WHITE][4][2] = A1 | D1;
    cached->castling_rookmask[WHITE][4][6] = F1 | H1;

    cached->castling_rookmask[BLACK][7* 8 + 4][7 * 8 + 2] = A8 | D8;
    cached->castling_rookmask[BLACK][7* 8 + 4][7 * 8 + 6] = F8 | H8;

    cached->OO[0] = G1 | F1;
    cached->OO[1] = G8 | F8;
    cached->OOO[0] = B1 | C1 | D1;
    cached->OOO[1] = B8 | C8 | D8;

    cached->OOto[WHITE] = 6;
    cached->OOto[BLACK] = 62;
    cached->OOOto[WHITE] = 2;
    cached->OOOto[BLACK] = 58;

    cached->OOOsq[WHITE][0] = 2;
    cached->OOOsq[WHITE][1] = 3;
    cached->OOOsq[WHITE][2] = 4;
    cached->OOOsq[BLACK][0] = 58;
    cached->OOOsq[BLACK][1] = 59;
    cached->OOOsq[BLACK][2] = 60;

    cached->OOsq[WHITE][0] = 4;
    cached->OOsq[WHITE][1] = 5;
    cached->OOsq[WHITE][2] = 6;
    cached->OOsq[BLACK][0] = 60;
    cached->OOsq[BLACK][1] = 61;
    cached->OOsq[BLACK][2] = 62;

    int y, x, y2, x2;
    int i;
    int ydir, xdir, dir;
    for (y = 0; y < 8; ++y)
    {
        for (x = 0; x < 8; ++x)
        {
            int idx = y * 8 + x;

            /* PAWN */
            if (y > 0 && y < 7)
            {
                cached->moves_pawn_one[WHITE][idx] |= 1ull << (idx + 8);
                if (y == 1)
                {
                    cached->moves_pawn_two[WHITE][idx] |= 1ull << (idx + 16);
                }

                cached->moves_pawn_one[BLACK][idx] |= 1ull << (idx - 8);
                if (y == 6)
                {
                    cached->moves_pawn_two[BLACK][idx] |= 1ull << (idx - 16);
                }

                cached->moves_pawn_two[WHITE][idx] |= cached->moves_pawn_one[WHITE][idx];
                cached->moves_pawn_two[BLACK][idx] |= cached->moves_pawn_one[BLACK][idx];

                if (x < 7)
                {
                    cached->attacks_pawn[WHITE][idx] |= 1ull << (idx + 9);
                    cached->attacks_pawn[BLACK][idx] |= 1ull << (idx - 7);
                }
                if (x > 0)
                {
                    cached->attacks_pawn[WHITE][idx] |= 1ull << (idx + 7);
                    cached->attacks_pawn[BLACK][idx] |= 1ull << (idx - 9);
                }
            }

            if (y > 1)
            {
                if (x < 7)
                {
                    cached->attacked_by_pawn[WHITE][idx] |= 1ull << (idx - 7);
                }
                if (x > 0)
                {
                    cached->attacked_by_pawn[WHITE][idx] |= 1ull << (idx - 9);
                }
            }
            if (y < 6)
            {
                if (x < 7)
                {
                    cached->attacked_by_pawn[BLACK][idx] |= 1ull << (idx + 9);
                }
                if (x > 0)
                {
                    cached->attacked_by_pawn[BLACK][idx] |= 1ull << (idx + 7);
                }
            }

            int knight_arr[8][2] = {
                {2, 1}, {1, 2}, {-1, 2}, {-2, 1},
                {-2, -1}, {-1, -2}, {1, -2}, {2, -1}
            };

            for (i = 0; i < 8; ++i)
            {
                y2 = y + knight_arr[i][0];
                x2 = x + knight_arr[i][1];

                if (y2 >= 0 && x2 >= 0 && y2 <= 7 && x2 <= 7)
                {
                    cached->moves_knight[idx] |= 1ull << (y2 * 8 + x2);
                }
            }

            int bishop_arr[4][3] = {
                {1, 1, NE}, {1, -1, NW},
                {-1, 1, SE}, {-1, -1, SW}
            };

            for (i = 0; i < 4; ++i)
            {
                ydir = bishop_arr[i][0];
                xdir = bishop_arr[i][1];
                dir = bishop_arr[i][2];

                y2 = y + ydir;
                x2 = x + xdir;

                while (y2 >= 0 && x2 >= 0 && y2 <= 7 && x2 <= 7)
                {
                    cached->directions[dir][idx] |= 1ull << (y2 * 8 + x2);
                    y2 += ydir;
                    x2 += xdir;
                }

                cached->moves_bishop[idx] |= cached->directions[dir][idx];
            }

            int rook_arr[4][3] = {
                {1, 0, NORTH}, {0, 1, EAST},
                {-1, 0, SOUTH}, {0, -1, WEST}
            };

            for (i = 0; i < 4; ++i)
            {
                ydir = rook_arr[i][0];
                xdir = rook_arr[i][1];
                dir = rook_arr[i][2];

                y2 = y + ydir;
                x2 = x + xdir;

                while (y2 >= 0 && x2 >= 0 && y2 <= 7 && x2 <= 7)
                {
                    cached->directions[dir][idx] |= 1ull << (y2 * 8 + x2);
                    y2 += ydir;
                    x2 += xdir;
                }

                cached->moves_rook[idx] |= cached->directions[dir][idx];
            }

            int king_arr[8][2] = {
                {1, 0}, {1, 1}, {0, 1}, {-1, 1},
                {-1, 0}, {-1, -1}, {0, -1}, {1, -1}
            };

            for (i = 0; i < 8; ++i)
            {
                y2 = y + king_arr[i][0];
                x2 = x + king_arr[i][1];

                if (x2 >= 0 && y2 >= 0 && x2 <= 7 && y2 <= 7)
                {
                    cached->moves_king[idx] |= 1ull << (y2 * 8 + x2);
                }
            }
        }
    }
}

void cache_destroy()
{
    free(cached);
}
