#define _GNU_SOURCE
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "cache.h"
#include "defines.h"

void cache_init()
{
    /* Generates the cache from scratch.
     * Should only be called once. */
    cached = malloc(sizeof(cached_t));
    memset(cached, 0, sizeof(cached_t));

    cached->castling_by_color[0] = A1 | H1;
    cached->castling_by_color[1] = A8 | H8;

    cached->castling_steps[0][0] = C1 | D1;
    cached->castling_steps[0][1] = F1 | G1;
    cached->castling_steps[1][0] = C8 | D8;
    cached->castling_steps[1][1] = F8 | G8;

    cached->promotion_rank[0] = A8 | B8 | C8 | D8 | E8 | F8 | G8 | H8;
    cached->promotion_rank[1] = A1 | B1 | C1 | D1 | E1 | F1 | G1 | H1;

    cached->castling_availability[WHITE][0][ffsll(E1) - 1] = A1;
    cached->castling_availability[WHITE][1][ffsll(E1) - 1] = H1;
    cached->castling_availability[BLACK][0][ffsll(E8) - 1] = A8;
    cached->castling_availability[BLACK][1][ffsll(E8) - 1] = H8;

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
