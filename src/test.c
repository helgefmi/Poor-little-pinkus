#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <math.h>
#include "test.h"
#include "move.h"
#include "defines.h"
#include "state.h"

/* File with functions used for testing the engine; perft and divide are found here! */

uint64_t test_perft(state_t *state, int depth, int verbose)
{
    /* Given a position, it will recursivly apply every possible
     * move for a given depth and count the leaf nodes. */

    if (depth == 0)
    {
        int res = !move_is_attacked(state, state->pieces[1 - state->turn][KING], state->turn);
        return res;
    }

    move_t moves[100];
    int count = 0;

    move_generate_moves(state, moves, &count);

    /* Check for invalid position or board with no pieces :) */
    if (count <= 0)
    {
        return 0;
    }

    char move_str[16];
    uint64_t nodes = 0;

    while (count--)
    {
        move_make(state,  &moves[count]);

        uint64_t res = test_perft(state, depth - 1, 0);
        if (verbose && res > 0)
        {
            move_to_string(&moves[count], move_str);
            printf("%s: %lld\n", move_str, res);
        }

        nodes += res;

        move_unmake(state, &moves[count]);
    }

    return nodes;
}

void test_divide(state_t *state, int depth)
{
    struct timeval start_time;
    struct timeval now;
    gettimeofday(&start_time, NULL);

    uint64_t total_nodes = test_perft(state, depth, 1);

    gettimeofday(&now, NULL);

    double spent_time;
    spent_time = now.tv_sec - start_time.tv_sec;
    spent_time *= 1000000;
    spent_time += (now.tv_usec - start_time.tv_usec);
    spent_time /= 1000000.0;

    printf("Time: %f, Nodes: %llu, nps: %f\n", spent_time, total_nodes, total_nodes/spent_time);
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
            uint64_t answer = atoll(answer_str),
                     result = test_perft(&state, depth, 0);

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
        spent_time /= 1000000.0;

        printf("\tnps=%f\n", total_nodes / spent_time);
    }

    fclose(file);

    printf("\nFailed tests: %d/%d\n", errors, lineno);

    double total_time = now.tv_sec - start_time.tv_sec;
    total_time *= 1000000;
    total_time += (now.tv_usec - start_time.tv_usec);
    total_time /= 1000000.0;
    printf("%llu nodes in %.2f seconds with nps=%f\n", total_nodes, total_time, total_nodes / total_time);
}
