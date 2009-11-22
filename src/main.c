#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "defines.h"
#include "cache.h"
#include "state.h"
#include "test.h"
#include "hash.h"

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
"                    leaf-nodes counts.\n"
" -mode perft      - Prints out the number of leaf nodes of a position running\n"
"                    through every move with n depth.\n"
" -mode perftsuite - Runs through a set of 126 positions and compare the perft\n"
"                    values by their correct values. Runs with arbitrary depth.\n"
" -mode xboard     - Starts the engine in xboard mode.\n\n"
" Anything else prints our this message.\n");
}

#define MODE_PRINT_USAGE 0
#define MODE_XBOARD 1
#define MODE_PERFT 2
#define MODE_DIVIDE 3
#define MODE_PERFTSUITE 4

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
            else if (0 == strcmp(argv[i], "xboard"))
            {
                mode = MODE_XBOARD;
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

    printf("Initializing hash table with size: %d\n", table_size);
    hash_init(table_size);

    state_t *state = malloc(sizeof(state_t));
    state_init_from_fen(state, fen);

    switch (mode)
    {
        case MODE_XBOARD:
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
    }

    free(state);
    cache_destroy();
    hash_destroy();
    return 0;
}
