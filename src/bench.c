#include <sys/time.h>
#include <stdio.h>
#include "bench.h"
#include "state.h"
#include "timectrl.h"
#include "hash.h"
#include "util.h"
#include "search.h"

#define NUM_BENCHES 8
static char* benchmarks[NUM_BENCHES] = {
    "3r1k2/4npp1/1ppr3p/p6P/P2PPPP1/1NR5/5K2/2R5 w - - - -",
    "rnbqkb1r/p3pppp/1p6/2ppP3/3N4/2P5/PPP1QPPP/R1B1KB1R w KQkq - - -",
    "4b3/p3kp2/6p1/3pP2p/2pP1P2/4K1P1/P3N2P/8 w - - - -",
    "r3r1k1/ppqb1ppp/8/4p1NQ/8/2P5/PP3PPP/R3R1K1 b - - - -",
    "2r2rk1/1bqnbpp1/1p1ppn1p/pP6/N1P1P3/P2B1N1P/1B2QPP1/R2R2K1 w - - - -",
    "r1bqk2r/pp2bppp/2p5/3pP3/P2Q1P2/2N1B3/1PP3PP/R4RK1 b kq - - -",
    "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - - -",
    "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - - -"
};

int depths[NUM_BENCHES] = {
    12, 10, 16,
    11, 10, 10,
    9, 10
};

void bench_start(int depth)
{
    uint64_t ab_nodes = 0, qs_nodes = 0;
    struct timeval start_time, now;
    float cache_hits = 0,
          cache_misses = 0,
          eval_cache_hits = 0,
          eval_cache_misses = 0;

    gettimeofday(&start_time, 0);

    int i;
    for (i = 0; i < NUM_BENCHES; ++i)
    {
        state_t state;
        state_init_from_fen(&state, benchmarks[i]);

        hash_wipe();
        timectrl_go(&state, 0, 0, 0, depth ? depth : depths[i], 0, 0, 0);

        ab_nodes += search.visited_nodes;
        qs_nodes += search.qs_visited_nodes;
        cache_hits += search.cache_hits;
        cache_misses += search.cache_misses;
        eval_cache_hits += search.eval_cache_hits;
        eval_cache_misses += search.eval_cache_misses;

        gettimeofday(&now, 0);

        double spent_time;
        spent_time = now.tv_sec - start_time.tv_sec;
        spent_time *= 1000000;
        spent_time += (now.tv_usec - start_time.tv_usec);

        printf("Time: %.2f, Nodes: ab=%.2fM, qs=%.2fM, nps: %.2fM\n",
            spent_time / 1000000.0,
            ab_nodes / 1000000.0,
            qs_nodes / 1000000.0,
            (ab_nodes + qs_nodes) / spent_time);

        printf("Cache hitrate: tt=%.2f eval=%.2f\n",
            100 * (cache_hits / (cache_hits + cache_misses)),
            100 * (eval_cache_hits / (eval_cache_hits + eval_cache_misses)));
        printf("Pruned nodes: %d\n", search.pruned_nodes);
        
        util_print_pv();
        printf("\n\n");

        fflush(stdout);
    }
}
