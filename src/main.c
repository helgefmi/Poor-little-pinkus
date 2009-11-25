#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "defines.h"
#include "cache.h"
#include "state.h"
#include "test.h"
#include "hash.h"
#include "uci.h"
#include "eval.h"
#include "bench.h"

void print_usage()
{
    printf("\n"
"plp - the horrific chess engine!\n\n"
"plp takes the following arguments:\n"
" -fen <fen>       - Sets up plp with an initial position.\n"
"                    If omitted, the normal initial position in chess is used.\n"
" -depth <n>       - Sets the depth of perft, divide or perftsuite modes.\n"
" -tsize <n>       - Sets the hash table size.\n\n"
" -mode divide     - Runs through every move in a position prints their\n"
"                    leaf-node counts.\n"
" -mode perft      - Same as '-mode divide' but will only give one leaf-node\n"
"                    count for the starting position, not for each move.\n"
" -mode perftsuite - Runs through a set of 126 positions and compare the perft\n"
"                    values by their correct values. Runs with arbitrary depth.\n"
" -mode uci        - Starts the engine in uci mode.\n\n"
" -mode bench      - Starts a benchmark for our main search algorithm.\n\n"
" Anything else prints out this message.\n");
}

#define MODE_PRINT_USAGE 0
#define MODE_UCI 1
#define MODE_PERFT 2
#define MODE_DIVIDE 3
#define MODE_PERFTSUITE 4
#define MODE_BENCH 5

int main(int argc, char **argv)
{
    char *fen = FEN_INIT;
    int depth = 1;
    int table_size = 1024;
    int mode = MODE_PRINT_USAGE;

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
                mode = MODE_DIVIDE;
            }
            else if (0 == strcmp(argv[i], "perft"))
            {
                mode = MODE_PERFT;
            }
            else if (0 == strcmp(argv[i], "perftsuite"))
            {
                mode = MODE_PERFTSUITE;
            }
            else if (0 == strcmp(argv[i], "uci"))
            {
                mode = MODE_UCI;
            }
            else if (0 == strcmp(argv[i], "bench"))
            {
                mode = MODE_BENCH;
            }
            else
            {
                fprintf(stderr, "Invalid mode: %s\n", argv[i]);
                mode = MODE_PRINT_USAGE;
                break;
            }
        }
        else
        {
            fprintf(stderr, "Invalid option: %s\n", argv[i]);
            mode = MODE_PRINT_USAGE;
            break;
        }
    }

    srand(time(0));

    cache_init();
    hash_init();

#if 0
    state_t tmp_state;
    state_init_from_fen(&tmp_state, "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBN1 w KQkq - 0 1");
    printf("%d\n", eval_state(&tmp_state));
    exit(1);
#endif

    /* State will only be set up with -fen if we want perft/divide */
    state_t *state = 0;
    switch (mode)
    {
        case MODE_PERFT:
        case MODE_DIVIDE:
            state = malloc(sizeof(state_t));
            state_init_from_fen(state, fen);
        case MODE_PERFTSUITE:
        case MODE_BENCH:
            printf("Initializing hash table with size: %d\n", table_size);
            hash_set_tsize(table_size);
            break;
    }

    switch (mode)
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
            bench_start(0);
            break;
    }

    free(state);
    cache_destroy();
    hash_destroy();
    return 0;
}
