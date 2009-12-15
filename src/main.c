#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "cache.h"
#include "state.h"
#include "test.h"
#include "hash.h"
#include "uci.h"
#include "eval.h"
#include "bench.h"
#include "plp.h"
#include "move.h"
#include "make.h"

int plp_mode;
void print_usage()
{
    printf("\n"
"plp - the horrific chess engine!\n\n"
"plp takes the following arguments:\n"
" -fen <fen>       - Sets up plp with an initial position.\n"
"                    If omitted, the normal startingposition in chess is used.\n"
" -depth <n>       - Sets the depth of perft, divide or perftsuite modes.\n"
" -tsize <n>       - Sets the hash table size (in MB).\n\n"
" -mode divide     - Runs through every move in a position and print their\n"
"                    leaf-node counts.\n"
" -mode perft      - Same as '-mode divide' but will only give one leaf-node\n"
"                    count for the starting position, not for each move.\n"
" -mode perftsuite - Runs through a set of 126 positions and compare the perft\n"
"                    values by their correct values. Runs with arbitrary depth.\n"
" -mode uci        - Starts the engine in uci mode. This is the default behaviour.\n\n"
" -mode bench      - Starts a benchmark for our main search algorithm.\n\n"
"\n");
}

int main(int argc, char **argv)
{
    char *fen = FEN_INIT;
    int depth = 0;
    int table_size = 512;
    plp_mode = MODE_UCI;

    int i;
    for (i = 1; i < argc; ++i)
    {
        if (0 == strcmp(argv[i], "-fen"))
        {
            fen = argv[++i];
        }
        else if (0 == strcmp(argv[i], "-depth"))
        {
            depth = atoi(argv[++i]);
        }
        else if (0 == strcmp(argv[i], "-tsize"))
        {
            table_size = atoi(argv[++i]);
        }
        else if (0 == strcmp(argv[i], "-mode"))
        {
            ++i;
            if (0 == strcmp(argv[i], "divide"))
            {
                plp_mode = MODE_DIVIDE;
            }
            else if (0 == strcmp(argv[i], "perft"))
            {
                plp_mode = MODE_PERFT;
            }
            else if (0 == strcmp(argv[i], "perftsuite"))
            {
                plp_mode = MODE_PERFTSUITE;
            }
            else if (0 == strcmp(argv[i], "uci"))
            {
                plp_mode = MODE_UCI;
            }
            else if (0 == strcmp(argv[i], "bench"))
            {
                plp_mode = MODE_BENCH;
            }
            else
            {
                fprintf(stderr, "Invalid mode: %s\n", argv[i]);
                plp_mode = MODE_PRINT_USAGE;
                break;
            }
        }
        else
        {
            fprintf(stderr, "Invalid option: %s\n", argv[i]);
            plp_mode = MODE_PRINT_USAGE;
            break;
        }
    }

    srand(5);

    cache_init();
    hash_init();

#if 0
    state_t tmp_state;
    state_init_from_fen(&tmp_state, "4k3/3pq3/bn2pnp1/2pP4/2B5/8/8/4K3 w - c6 0 1");
    //make_move(&tmp_state, PackMove(LSB(C7), LSB(C5), PAWN, -1, -1), 99);
    state_print(&tmp_state);
    test_perft(&tmp_state, 2, 1);
    exit(1);
#endif

    /* State will only be set up with -fen if we want perft/divide */
    state_t *state = 0;
    switch (plp_mode)
    {
        case MODE_PERFT:
        case MODE_DIVIDE:
            state = malloc(sizeof(state_t));
            state_init_from_fen(state, fen);
        case MODE_PERFTSUITE:
        case MODE_BENCH:
            printf("Initializing hash table with size: %dMB\n", table_size);
            hash_set_tsize(table_size);
            break;
    }

    switch (plp_mode)
    {
        case MODE_UCI:
            uci_start();
            break;
        case MODE_PERFTSUITE:
            test_perftsuite(depth);
            break;
        case MODE_PERFT:
            state_print(state);
            test_perft(state, depth, 0);
            break;
        case MODE_DIVIDE:
            state_print(state);
            test_perft(state, depth, 1);
            break;
        case MODE_PRINT_USAGE:
            print_usage();
            break;
        case MODE_BENCH:
            bench_start(depth);
            break;
    }

    free(state);
    cache_destroy();
    hash_destroy();
    return 0;
}
