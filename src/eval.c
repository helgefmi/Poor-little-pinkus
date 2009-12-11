#include "eval.h"
#include "plp.h"
#include "state.h"
#include "hash.h"

int eval_piece_values[6] = {100, 300, 310, 500, 900, 0};
int eval_real_pvalues[6] = {100, 300, 300, 500, 900, 44444};

static int pawn_pcsq[2][64] = {
     {0,   0,   0,   0,   0,   0,   0,   0,
     -6,  -4,   1, -24, -24,   1,  -4,  -6,
     -4,  -4,   1,   5,   5,   1,  -4,  -4,
     -6,  -4,   5,  10,  10,   5,  -4,  -6,
     -6,  -4,   2,   8,   8,   2,  -4,  -6,
     -6,  -4,   1,   2,   2,   1,  -4,  -6,
     -6,  -4,   1,   1,   1,   1,  -4,  -6,
      0,   0,   0,   0,   0,   0,   0,   0},

     {0,   0,   0,   0,   0,   0,   0,   0,
     -6,  -4,   1,   1,   1,   1,  -4,  -6,
     -6,  -4,   1,   2,   2,   1,  -4,  -6,
     -6,  -4,   2,   8,   8,   2,  -4,  -6,
     -6,  -4,   5,  10,  10,   5,  -4,  -6,
     -4,  -4,   1,   5,   5,   1,  -4,  -4,
     -6,  -4,   1, -24, -24,   1,  -4,  -6,
      0,   0,   0,   0,   0,   0,   0,   0}
};

static int knight_pcsq[2][64] = {
    {-8,  -12, -8,  -8,  -8,  -8, -12,  -8
     -8,   0,   1,   2,   2,   1,   0,  -8,
     -8,   0,   4,   4,   4,   4,   0,  -8,
     -8,   0,   4,   8,   8,   4,   0,  -8,
     -8,   0,   4,   8,   8,   4,   0,  -8,
     -8,   0,   4,   4,   4,   4,   0,  -8,
     -8,   0,   0,   0,   0,   0,   0,  -8,
     -8,  -8,  -8,  -8,  -8,  -8,  -8,  -8},

    {-8,  -8,  -8,  -8,  -8,  -8,  -8,  -8,
     -8,   0,   0,   0,   0,   0,   0,  -8,
     -8,   0,   4,   4,   4,   4,   0,  -8,
     -8,   0,   4,   8,   8,   4,   0,  -8,
     -8,   0,   4,   8,   8,   4,   0,  -8,
     -8,   0,   4,   4,   4,   4,   0,  -8,
     -8,   0,   1,   2,   2,   1,   0,  -8,
     -8,  -12, -8,  -8,  -8,  -8, -12,  -8}
};

static int bishop_pcsq[2][64] = {
    {-4,  -4, -12,  -4,  -4, -12,  -4,  -4
     -4,   2,   1,   1,   1,   1,   2,  -4,
     -4,   1,   2,   4,   4,   2,   1,  -4,
     -4,   0,   4,   6,   6,   4,   0,  -4,
     -4,   0,   4,   6,   6,   4,   0,  -4,
     -4,   0,   2,   4,   4,   2,   0,  -4,
     -4,   0,   0,   0,   0,   0,   0,  -4,
     -4,  -4,  -4,  -4,  -4,  -4,  -4,  -4},

    {-4,  -4,  -4,  -4,  -4,  -4,  -4,  -4,
     -4,   0,   0,   0,   0,   0,   0,  -4,
     -4,   0,   2,   4,   4,   2,   0,  -4,
     -4,   0,   4,   6,   6,   4,   0,  -4,
     -4,   0,   4,   6,   6,   4,   0,  -4,
     -4,   1,   2,   4,   4,   2,   1,  -4,
     -4,   2,   1,   1,   1,   1,   2,  -4,
     -4,  -4, -12,  -4,  -4, -12,  -4,  -4}
};

static int rook_pcsq[2][64] = {
     {0,   0,   0,   2,   2,   0,   0,   0
     -5,   0,   0,   0,   0,   0,   0,  -5,
     -5,   0,   0,   0,   0,   0,   0,  -5,
     -5,   0,   0,   0,   0,   0,   0,  -5,
     -5,   0,   0,   0,   0,   0,   0,  -5,
     -5,   0,   0,   0,   0,   0,   0,  -5,
     20,  20,  20,  20,  20,  20,  20,  20,
      5,   5,   5,   5,   5,   5,   5,   5},

     {5,   5,   5,   5,   5,   5,   5,   5,
     20,  20,  20,  20,  20,  20,  20,  20,
     -5,   0,   0,   0,   0,   0,   0,  -5,
     -5,   0,   0,   0,   0,   0,   0,  -5,
     -5,   0,   0,   0,   0,   0,   0,  -5,
     -5,   0,   0,   0,   0,   0,   0,  -5,
     -5,   0,   0,   0,   0,   0,   0,  -5,
      0,   0,   0,   2,   2,   0,   0,   0}
};

static int queen_pcsq[2][64] = {
    {-5,  -5,  -5,  -5,  -5,  -5,  -5,  -5,
      0,   0,   1,   1,   1,   1,   0,   0,
      0,   0,   1,   2,   2,   1,   0,   0,
      0,   0,   2,   3,   3,   2,   0,   0,
      0,   0,   2,   3,   3,   2,   0,   0,
      0,   0,   1,   2,   2,   1,   0,   0,
      0,   0,   1,   1,   1,   1,   0,   0,
      0,   0,   0,   0,   0,   0,   0,   0},

     {0,   0,   0,   0,   0,   0,   0,   0,
      0,   0,   1,   1,   1,   1,   0,   0,
      0,   0,   1,   2,   2,   1,   0,   0,
      0,   0,   2,   3,   3,   2,   0,   0,
      0,   0,   2,   3,   3,   2,   0,   0,
      0,   0,   1,   2,   2,   1,   0,   0,
      0,   0,   1,   1,   1,   1,   0,   0,
     -5,  -5,  -5,  -5,  -5,  -5,  -5,  -5}
};

static int king_pcsq[2][64] = {
     {0,  20,  30, -30,   0, -20,  30,  20,
    -15, -15, -20, -20, -20, -20, -15, -15,
    -40, -40, -40, -40, -40, -40, -40, -40,
    -40, -40, -40, -40, -40, -40, -40, -40,
    -40, -40, -40, -40, -40, -40, -40, -40,
    -40, -40, -40, -40, -40, -40, -40, -40,
    -40, -40, -40, -40, -40, -40, -40, -40,
    -40, -40, -40, -40, -40, -40, -40, -40},
   {-40, -40, -40, -40, -40, -40, -40, -40,
    -40, -40, -40, -40, -40, -40, -40, -40,
    -40, -40, -40, -40, -40, -40, -40, -40,
    -40, -40, -40, -40, -40, -40, -40, -40,
    -40, -40, -40, -40, -40, -40, -40, -40,
    -40, -40, -40, -40, -40, -40, -40, -40,
    -15, -15, -20, -20, -20, -20, -15, -15,
      0,  20,  30, -30,   0, -20,  30,  20}
};
 
static int king_pcsq_eg[2][64] = {
     {0,  10,  20,  30,  30,  20,  10,   0,
     10,  20,  30,  40,  40,  30,  20,  10,
     20,  30,  40,  50,  50,  40,  30,  20,
     30,  40,  50,  60,  60,  50,  40,  30,
     30,  40,  50,  60,  60,  50,  40,  30,
     20,  30,  40,  50,  50,  40,  30,  20,
     10,  20,  30,  40,  40,  30,  20,  10,
      0,  10,  20,  30,  30,  20,  10,   0},

     {0,  10,  20,  30,  30,  20,  10,   0,
     10,  20,  30,  40,  40,  30,  20,  10,
     20,  30,  40,  50,  50,  40,  30,  20,
     30,  40,  50,  60,  60,  50,  40,  30,
     30,  40,  50,  60,  60,  50,  40,  30,
     20,  30,  40,  50,  50,  40,  30,  20,
     10,  20,  30,  40,  40,  30,  20,  10,
      0,  10,  20,  30,  30,  20,  10,   0}
};

static int eval_pawns(state_t *state, int color)
{
    register uint64_t pawns = state->pieces[color][PAWN];
    register int ret = 0;

    for (; pawns; ClearLow(pawns))
    {
        int from = LSB(pawns);
        ret += pawn_pcsq[color][from];
    }

    return ret;
}

static int eval_knights(state_t *state, int color)
{
    register uint64_t knights = state->pieces[color][KNIGHT];
    register int ret = 0;

    for (; knights; ClearLow(knights))
    {
        int from = LSB(knights);
        ret += knight_pcsq[color][from];
    }

    return ret;
}

static int eval_bishops(state_t *state, int color)
{
    register uint64_t bishops = state->pieces[color][BISHOP];
    register int ret = 0;

    for (; bishops; ClearLow(bishops))
    {
        int from = LSB(bishops);
        ret += bishop_pcsq[color][from];
    }

    return ret;
}

static int eval_rooks(state_t *state, int color)
{
    register uint64_t rooks = state->pieces[color][ROOK];
    register int ret = 0;

    for (; rooks; ClearLow(rooks))
    {
        int from = LSB(rooks);
        ret += rook_pcsq[color][from];
    }

    return ret;
}

static int eval_queens(state_t *state, int color)
{
    register uint64_t queens = state->pieces[color][QUEEN];
    register int ret = 0;

    for (; queens; ClearLow(queens))
    {
        int from = LSB(queens);
        ret += queen_pcsq[color][from];
    }

    return ret;
}

static int eval_kings(state_t *state, int color)
{
    int ret = king_pcsq[color][state->king_idx[color]];
    return ret;
}

static int eval_kings_eg(state_t *state, int color)
{
    int ret = king_pcsq_eg[color][state->king_idx[color]];
    return ret;
}

static int eval_material(state_t *state, int color)
{
    register int ret = 0;
    register int *piecev = eval_piece_values, *end = eval_piece_values + KING + 1;
    register uint64_t *piece = state->pieces[color];

    for (; piecev < end; ++piecev, ++piece)
    {
        ret += *piecev * PopCnt(*piece);
    }

    return ret;
}

int eval_state(state_t *state)
{
    int ret;

#ifdef USE_HASH_EVAL
    if (!hash_get_eval(state->zobrist, &ret))
    {
#endif
        int wmaterial = eval_material(state, WHITE);
        int bmaterial = eval_material(state, BLACK);

        int is_endgame = wmaterial < 1300 && bmaterial < 1300;

        ret = wmaterial - bmaterial;
        ret += eval_pawns(state, WHITE) - eval_pawns(state, BLACK);
        ret += eval_knights(state, WHITE) - eval_knights(state, BLACK);
        ret += eval_bishops(state, WHITE) - eval_bishops(state, BLACK);
        ret += eval_rooks(state, WHITE) - eval_rooks(state, BLACK);
        ret += eval_queens(state, WHITE) - eval_queens(state, BLACK);

        if (is_endgame)
        {
            ret += eval_kings_eg(state, WHITE) - eval_kings_eg(state, BLACK);
        }
        else
        {
            ret += eval_kings(state, WHITE) - eval_kings(state, BLACK);
        }

        if (state->turn != WHITE)
            ret = -ret;

#ifdef USE_HASH_EVAL
        hash_add_eval(state->zobrist, ret);
    }
#endif

    return ret;
}

int eval_quick(state_t *state)
{
    int ret;

#ifdef USE_HASH_EVAL
    if (hash_get_eval(state->zobrist, &ret))
        return ret;
#endif

    ret = eval_material(state, WHITE) - eval_material(state, BLACK);
    return state->turn == WHITE ? ret : -ret;
}
