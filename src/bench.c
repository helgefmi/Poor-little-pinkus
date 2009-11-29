#include <sys/time.h>
#include <stdio.h>
#include "bench.h"
#include "state.h"
#include "timectrl.h"
#include "hash.h"
#include "search.h"

#define NUM_BENCHES 6
static char* benchmarks[NUM_BENCHES] = {
    "3r1k2/4npp1/1ppr3p/p6P/P2PPPP1/1NR5/5K2/2R5 w - - - -",
    "rnbqkb1r/p3pppp/1p6/2ppP3/3N4/2P5/PPP1QPPP/R1B1KB1R w KQkq - - -",
    "4b3/p3kp2/6p1/3pP2p/2pP1P2/4K1P1/P3N2P/8 w - - - -",
    "r3r1k1/ppqb1ppp/8/4p1NQ/8/2P5/PP3PPP/R3R1K1 b - - - -",
    "2r2rk1/1bqnbpp1/1p1ppn1p/pP6/N1P1P3/P2B1N1P/1B2QPP1/R2R2K1 w - - - -",
    "r1bqk2r/pp2bppp/2p5/3pP3/P2Q1P2/2N1B3/1PP3PP/R4RK1 b kq - - -"
};

int depths[NUM_BENCHES] = {
    11, 9, 14,
    9, 9, 9
};

void bench_start(int depth)
{
    uint64_t total_nodes = 0;
    struct timeval start_time, now;
    float cache_hits = 0,
          cache_misses = 0;

    gettimeofday(&start_time, 0);

    int i;
    for (i = 0; i < NUM_BENCHES; ++i)
    {
        state_t state;
        state_init_from_fen(&state, benchmarks[i]);

        hash_wipe();
        timectrl_go(&state, 0, 0, 0, depth ? depth : depths[i], 0, 0, 0);

        total_nodes += search.visited_nodes;
        cache_hits += search.cache_hits;
        cache_misses += search.cache_misses;

        printf(".");
        fflush(stdout);
    }

    gettimeofday(&now, 0);

    double spent_time;
    spent_time = now.tv_sec - start_time.tv_sec;
    spent_time *= 1000000;
    spent_time += (now.tv_usec - start_time.tv_usec);

    printf("\n\nTime: %f, Nodes: %.2fM, nps: %.2fM\n", spent_time / 1000000.0, total_nodes / 1000000.0, total_nodes/spent_time);
    printf("Cache hitrate: %.2f\n", 100 * (cache_hits / (cache_hits + cache_misses)));
}
