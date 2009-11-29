#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <math.h>
#include "test.h"
#include "move.h"
#include "state.h"
#include "hash.h"
#include "plp.h"
#include "util.h"
#include "make.h"

static uint64_t _cache_hits = 0, _cache_misses = 0;
uint64_t test_perft_rec(state_t *state, int depth, int verbose)
{
    /* Given a position, it will recursivly apply every possible
     * move for a given depth and count the leaf nodes. */

    if (depth == 0)
    {
        return 1;
    }

    hash_node_t *hash_node = hash_get_node(state->zobrist);
    /* Check if hashing is enabled */
    if (hash_node)
    {
        if (hash_node->hash == state->zobrist && hash_node->depth == depth)
        {
            ++_cache_hits;
            return hash_node->score;
        }
        else
        {
            ++_cache_misses;
        }
    }

    int moves[100];
    int count = 0, count2 = 0;

    move_generate_moves(state, moves, &count);
    move_generate_tactical(state, moves + count, &count2);
    count += count2;

    /* Check for invalid position or board with no pieces :) */
    if (count <= 0)
    {
        return 0;
    }

    uint64_t nodes = 0;

    int i;
    for (i = 0; i < count; ++i)
    {
        make_move(state, moves[i], depth);

        if (move_is_attacked(state, state->king_idx[Flip(state->turn)], state->turn))
        {
            unmake_move(state, moves[i], depth);
            continue;
        }

        uint64_t res = test_perft_rec(state, depth - 1, 0);
        nodes += res;

        if (verbose && res > 0)
        {
            char move_str[16];
            util_move_to_lan(moves[i], move_str);
            printf("%s: %lld\n", move_str, res);
        }

        unmake_move(state, moves[i], depth);
    }

    hash_add_node(state->zobrist, nodes, depth, 0, 0);

    return nodes;
}

void test_perft(state_t *state, int depth, int divide)
{
    /* Used for both -mode perft and -mode divide */
    struct timeval start_time;
    struct timeval now;
    gettimeofday(&start_time, NULL);

    uint64_t total_nodes = test_perft_rec(state, depth, divide);

    gettimeofday(&now, NULL);

    double spent_time;
    spent_time = now.tv_sec - start_time.tv_sec;
    spent_time *= 1000000;
    spent_time += (now.tv_usec - start_time.tv_usec);

    printf("Seconds: %.2f, Nodes: %llu, nps: %.2fM\n", spent_time / 1000000.0, total_nodes, total_nodes/spent_time);
    printf("Cache hits: %llu, Cache misses: %llu\n", _cache_hits, _cache_misses);
}

void test_perftsuite(int max_depth)
{
    /* Runs 126 startion position through perft() and checks if the nodecount is correct
     * This tests the move generation and make/unmake moves of moggio, and should always
     * have 0/126 failed. */

    FILE *file = fopen("perftsuite.epd", "r");

    if (!file)
    {
        printf("Couldn't open perftsuite.epd. Cannot continue perftsuite.\n");
        return;
    }

    state_t state;

    int lineno = 0,
        errors = 0;

    uint64_t total_nodes = 0;
 
    char buf[1024];

    struct timeval start_time, now;
    gettimeofday(&start_time, NULL);
    while (fgets(buf, 1024, file))
    {
        char *fen = strtok(buf, ";");
        state_init_from_fen(&state, fen);

        int depth = 1;
        char *answer_str;

        printf("%d:", ++lineno);

        while (depth <= max_depth && (answer_str = strtok(NULL, ";")))
        {
            uint64_t answer = atoll(answer_str);

            uint64_t result = test_perft_rec(&state, depth, 0);

            total_nodes += result;

            printf("\t%d=%llu", depth, result);

            if (answer != result)
            {
                printf("\tFAIL! diff=%llu depth=%u me=%llu correct=%llu fen=%s", llabs(result - answer), depth, result, answer, fen);
                ++errors;
                break;
            }
            ++depth;

            fflush(stdout);
        }

        gettimeofday(&now, NULL);
        double spent_time = now.tv_sec - start_time.tv_sec;
        spent_time *= 1000000;
        spent_time += (now.tv_usec - start_time.tv_usec);

        printf("\tnps=%.2fM\n", total_nodes / spent_time);
    }

    fclose(file);

    printf("\nFailed tests: %d/%d\n", errors, lineno);

    double total_time = now.tv_sec - start_time.tv_sec;
    total_time *= 1000000;
    total_time += (now.tv_usec - start_time.tv_usec);

    printf("%llu nodes in %.2f seconds with nps=%.2fM\n", total_nodes, total_time / 1000000.0, total_nodes / total_time);
    printf("Cache hits: %llu, Cache misses: %llu\n", _cache_hits, _cache_misses);
}
