#include <unistd.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include "uci.h"
#include "util.h"
#include "hash.h"
#include "plp.h"
#include "search.h"
#include "move.h"
#include "timectrl.h"

#define DEFAULT_HASH_SIZE 256
#define MAX_HASH_SIZE 2048

static int uci_active = 0;
static state_t *uci_state = 0;
static FILE *uci_logfile;

void uci_start()
{
    uci_logfile = fopen("/home/helge/output.txt", "w");
    uci_state = malloc(sizeof(state_t));

    uci_active = 1;
    while (uci_active)
    {
        uci_input();
    }
}

void uci_input()
{
    uci_debug("Getting input..");

    char buf[4096];

    if (!fgets(buf, 4096, stdin))
    {
        uci_debug("fgets returned 0..");
        uci_quit();
    }
    else
    {
        char *cmd = util_trim_str(buf);
        uci_parse_cmd(cmd);
    }
}

void uci_quit()
{
    free(uci_state);
    fclose(uci_logfile);
    uci_active = 0;
}

void uci_parse_cmd(char *cmd)
{
    uci_debug(cmd);

    if (!strcmp(cmd, "quit"))
    {
        uci_quit();
    }
    else if (!strcmp(cmd, "stop"))
    {
        uci_halt_search();
        uci_bestmove();
    }
    else if (!strcmp(cmd, "isready"))
    {
        printf("readyok\n");
        fflush(stdout);
    }
    else if (!strncmp(cmd, "setoption name Hash value ", strlen("setoption name Hash value ")))
    {
        int value = atoi(cmd + strlen("setoption name Hash value "));
        uci_set_hash_size(value);
    }
    else if (!strcmp(cmd, "ucinewgame"))
    {
        uci_halt_search();
        hash_wipe();
    }
    else if (!strncmp(cmd, "go", 2))
    {
        uci_go(cmd + 3);
    }
    else if (!strncmp(cmd, "position", 8))
    {
        uci_halt_search();
        uci_init_position(cmd + 9);
    }
    else if (!strcmp(cmd, "uci"))
    {
        printf("id name Poor little Pinkus\n");
        printf("id author Helge Milde\n");
        printf("option name Hash type spin default %d min 1 max %d\n", DEFAULT_HASH_SIZE, MAX_HASH_SIZE);
        uci_set_hash_size(DEFAULT_HASH_SIZE);
        printf("uciok\n");
        fflush(stdout);
    }
}

void uci_halt_search()
{
    if (timecontrol.searching)
    {
        uci_debug("Halting search.");
        timecontrol.searching = 0;
    }
}

void uci_bestmove()
{
    if (search_data.pv[0].depth <= 0)
    {
        uci_debug("Didn't have a best move in bestmove() :-(");
        printf("bestmove 0000\n");
        fflush(stdout);
    }
    else
    {
        char buf[16];
        move_to_string(&search_data.pv[0].move, buf);
        uci_debug("giving out best move:");
        uci_debug(buf);
        printf("bestmove %s\n", buf);
        fflush(stdout);
    }
}

void uci_init_position(char *position)
{
    memset(uci_state, 0, sizeof(state_t));

    char word[4096];
    sscanf(position, "%s", word);
    position += strlen(word) + 1;

    if (!strcmp(word, "startpos"))
    {
        state_init_from_fen(uci_state, FEN_INIT);
    }
    else if (!strcmp(word, "fen"))
    {
        int used_chars = state_init_from_fen(uci_state, position);
        position += used_chars;
    }

    while (isspace(position[0]))
    {
        ++position;
    }

    if (position[0])
    {
        sscanf(position, "%s", word);
        position += strlen(word) + 1;

        if (!strcmp(word, "moves"))
        {
            while (position[0])
            {
                sscanf(position, "%s", word);
                position += strlen(word) + 1;

                move_t move;
                util_chars_to_move(word, &move, uci_state);
                move_make(uci_state, &move, 0);
            }
        }
    }

    state_print(uci_state);
}

void uci_set_hash_size(int value)
{
    uci_debug("Setting hash table size..");
    hash_set_tsize(value);
}

void uci_debug(char *str)
{
    fprintf(uci_logfile, "%d: %s\n", getpid(), str);
    fflush(uci_logfile);
}

void uci_go(char *params)
{
    int wtime = 0, btime = 0,
        ponder = 0, depth = 0,
        nodes = 0, infinite = 0;

    while (params[0])
    {
        char word[4096];
        sscanf(params, "%s", word);
        params += strlen(word) + 1;

        if (!strcmp(word, "wtime"))
        {
            sscanf(params, "%s", word);
            params += strlen(word) + 1;
            wtime = atoi(word);
        }
        else if (!strcmp(word, "btime"))
        {
            sscanf(params, "%s", word);
            params += strlen(word) + 1;
            btime = atoi(word);
        }
        else if (!strcmp(word, "ponder"))
        {
            ponder = 1;
        }
        else if (!strcmp(word, "depth"))
        {
            sscanf(params, "%s", word);
            params += strlen(word) + 1;
            depth = atoi(word);
        }
        else if (!strcmp(word, "nodes"))
        {
            sscanf(params, "%s", word);
            params += strlen(word) + 1;
            nodes = atoi(word);
        }
        else if (!strcmp(word, "infinite"))
        {
            infinite = 1;
        }

        while (isspace(params[0]))
        {
            ++params;
        }
    }

    timectrl_go(uci_state, wtime, btime, ponder, depth, nodes, infinite, 1);

    uci_bestmove();
}
