#ifndef _CACHE_H
#define _CACHE_H

#include <stdint.h>

typedef struct
{
    /* Since pawn are special in that the moves are different from each color
     * and also because it sometimes can move one and other times two steps,
     * we use three variables to cache the moves. */
    uint64_t attacks_pawn[2][64],
             moves_pawn_one[2][64],
             moves_pawn_two[2][64],

    /* Kinda the inverse of attacks_pawn; the bits says where a pawn must be positioned
     * to attack a certain square */
            attacked_by_pawn[2][64],

    /* Knight and king moves are very easy to calculate, and only needs one dictionary each. */
            moves_knight[64],
            moves_king[64],

    /* Bishop, Rooks and Queen moves can be combined from rays going in every direction. */
            directions[8][65],

    /* We use "directions" to calculate these as well */
            moves_bishop[64],
            moves_rook[64],

    /* Used to efficiently find out if castling is available */
            castling_availability[2][2][64],
            castling_by_color[2],
            castling_rookmask[2][64][64],

    /* Used to efficiently find out if there are any pieces blocking a castle */
            castling_steps[2][2],

    /* Used to efficiently find out if a pawn can promote (basically set to
    * rank #8 for white and rank #1 for black) */
            promotion_rank[2];

    int piece_to_int[256];
    char int_to_piece[2][6];

} cached_t;

cached_t *cached;

void cache_init();
void cache_destroy();

#endif
