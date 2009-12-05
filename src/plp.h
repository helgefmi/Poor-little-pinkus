#ifndef _PLP_H
#define _PLP_H

#define USE_HASH_EVAL

#define FEN_INIT "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"

#define WHITE 0
#define BLACK 1

#define PAWN 0
#define KNIGHT 1
#define BISHOP 2
#define ROOK 3
#define QUEEN 4
#define KING 5

#define NORTH 0
#define EAST 1
#define SOUTH 2
#define WEST 3
#define NW 4
#define NE 5
#define SE 6
#define SW 7

#define INF 99999999

#define MoveFrom(move)    (move & 63)
#define MoveTo(move)      ((move >> 6) & 63)
#define MovePiece(move)   ((move >> 12) & 7)
#define MoveCapture(move) ((move >> 15) & 7)
#define MovePromote(move) ((move >> 18) & 7)

#define Killer1(ply) search.killers[ply][0]
#define Killer2(ply) search.killers[ply][1]

#define PackMove(from, to, piece, capture, promote) (from | ((to & 63) << 6) | ((piece & 7) << 12) | ((capture & 7) << 15) | ((promote & 7) << 18))

#define ClearLow(x) ((x) &= (x) - 1)
#define Flip(x) ((x)^1)
#define Abs(x) ((x) >= 0 ? (x) : -(x))

/* Headers for optimized versions of LSB, MSB and PopCnt */
#if defined(__LP64__)
    #include "inline64.h"
#else
    #include "inline32.h"
#endif

/* A1 - H8 {{{ */
#define A1 1ull << 0
#define B1 1ull << 1
#define C1 1ull << 2
#define D1 1ull << 3
#define E1 1ull << 4
#define F1 1ull << 5
#define G1 1ull << 6
#define H1 1ull << 7
#define A2 1ull << 8
#define B2 1ull << 9
#define C2 1ull << 10
#define D2 1ull << 11
#define E2 1ull << 12
#define F2 1ull << 13
#define G2 1ull << 14
#define H2 1ull << 15
#define A3 1ull << 16
#define B3 1ull << 17
#define C3 1ull << 18
#define D3 1ull << 19
#define E3 1ull << 20
#define F3 1ull << 21
#define G3 1ull << 22
#define H3 1ull << 23
#define A4 1ull << 24
#define B4 1ull << 25
#define C4 1ull << 26
#define D4 1ull << 27
#define E4 1ull << 28
#define F4 1ull << 29
#define G4 1ull << 30
#define H4 1ull << 31
#define A5 1ull << 32
#define B5 1ull << 33
#define C5 1ull << 34
#define D5 1ull << 35
#define E5 1ull << 36
#define F5 1ull << 37
#define G5 1ull << 38
#define H5 1ull << 39
#define A6 1ull << 40
#define B6 1ull << 41
#define C6 1ull << 42
#define D6 1ull << 43
#define E6 1ull << 44
#define F6 1ull << 45
#define G6 1ull << 46
#define H6 1ull << 47
#define A7 1ull << 48
#define B7 1ull << 49
#define C7 1ull << 50
#define D7 1ull << 51
#define E7 1ull << 52
#define F7 1ull << 53
#define G7 1ull << 54
#define H7 1ull << 55
#define A8 1ull << 56
#define B8 1ull << 57
#define C8 1ull << 58
#define D8 1ull << 59
#define E8 1ull << 60
#define F8 1ull << 61
#define G8 1ull << 62
#define H8 1ull << 63
/* }}} */

#endif
