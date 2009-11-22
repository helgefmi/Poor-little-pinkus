#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "uci.h"
#include "util.h"
#include "hash.h"
#include "defines.h"
#include "move.h"

#define DEFAULT_HASH_SIZE 128

static move_t *uci_best_move = 0;
static state_t *uci_state = 0;

void uci_start()
{
    uci_set_hash_size(DEFAULT_HASH_SIZE);
    while (1)
    {
        uci_input();
    }
}

void uci_input()
{
    char buf[4096];
    if (0 == fgets(buf, 4096, stdin))
    {
        uci_quit(1);
    }

    char *cmd = util_trim_str(buf);
    uci_parse_cmd(cmd);
}

void uci_parse_cmd(char *cmd)
{
    if (0 == strcmp(cmd, "quit"))
    {
        uci_quit();
    }
    else if (0 == strcmp(cmd, "stop"))
    {
        uci_halt_search();
    }
    else if (0 == strcmp(cmd, "isready"))
    {
        printf("readyok\n");
    }
    else if (0 == memcmp(cmd, "setoption name Hash value ", strlen("setoption name Hash value ")))
    {
        int value = atoi(cmd + strlen("setoption name Hash value "));
        uci_set_hash_size(value);
    }
    else if (0 == strcmp(cmd, "ucinewgame"))
    {
        uci_halt_search();
        uci_new_game();
    }
    else if (0 == memcmp(cmd, "go", 2))
    {
    }
    else if (0 == memcmp(cmd, "position", 8))
    {
        uci_init_position(cmd + 9);
    }
    else if (0 == strcmp(cmd, "uci"))
    {
        printf("id name Poor little Pinkus\n");
        printf("id author Helge Milde\n");
        printf("option name Hash type spin default 128 min 1 max 512\n");
        printf("uciok\n");
    }
}

void uci_quit()
{
    exit(1);
}

void uci_halt_search()
{
    if (!uci_best_move)
    {
        printf("bestmove 0000\n");
    }
    else
    {
        char buf[16];
        move_to_string(uci_best_move, buf);
        printf("bestmove %s\n", buf);
    }
}

void uci_new_game()
{
    printf("info string Wiping hash tables\n");
    hash_wipe();

    if (uci_state)
    {
        printf("info string Freeing uci_state\n");
        free(uci_state);
        uci_state = 0;
    }

    if (uci_best_move)
    {
        printf("info string Freeing uci_best_move\n");
        free (uci_best_move);
        uci_best_move = 0;
    }
}
 
void uci_init_position(char *position)
{
    assert(!uci_state);
    assert(!uci_best_move);

    char word[4096];
    uci_state = malloc(sizeof(state_t));

    sscanf(position, "%s", word);
    position += strlen(word) + 1;

    if (0 == strcmp(word, "startpos"))
    {
        state_init_from_fen(uci_state, FEN_INIT);
    }
    else if (0 == strcmp(word, "fen"))
    {
        int used_chars = state_init_from_fen(uci_state, position);
        position += used_chars;
    }

    state_print(uci_state);

    while (isspace(position[0]))
    {
        ++position;
    }

    if (position[0])
    {

        printf("info string Unused part of newposition: '%s'\n", position);
    }
}

void uci_set_hash_size(int value)
{
    int tsize = (value * 1024 * 1024) / sizeof(hash_node_t);
    printf("info string Setting hash table size to %dMB (%d entries)\n", value, tsize);
    hash_set_tsize(tsize);
}
