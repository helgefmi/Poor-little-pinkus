#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <sys/wait.h>
#include "uci.h"
#include "util.h"
#include "hash.h"
#include "defines.h"
#include "search.h"
#include "move.h"

#define DEFAULT_HASH_SIZE 128

static int uci_childpid = 0;
static state_t *uci_state = 0;
static FILE *uci_logfile;

void uci_start()
{
    uci_logfile = fopen("/home/helge/output.txt", "w");

    uci_debug("Starting..");
    uci_set_hash_size(DEFAULT_HASH_SIZE);
    while (1)
    {
        uci_debug("Getting input..");
        uci_input();
    }
}

void uci_input()
{
    char buf[4096];
    if (0 == fgets(buf, 4096, stdin))
    {
        uci_debug("fgets returned 0..");
        uci_quit();
    }

    char *cmd = util_trim_str(buf);
    uci_debug(cmd);

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
        fflush(stdout);
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
        uci_go();
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
        fflush(stdout);
    }
}

void uci_quit()
{
    if (uci_childpid > 0)
    {
        uci_debug("Sending SIGKILL to childpid");
        kill(uci_childpid, SIGKILL);
        wait(0);
    }
    uci_debug("Quitting..");
    exit(0);
}

void uci_halt_search()
{
    if (uci_childpid)
    {
        uci_debug("Killing child in halt_search..");
        kill(uci_childpid, SIGUSR1);
        wait(0);
        uci_childpid = 0;
    }
    else
    {
        uci_debug("Didn't need to kill child in halt_search");
    }
}

void uci_bestmove()
{
    if (!search_best_move)
    {
        uci_debug("search_best_move == 0??");
    }

    if (0 == search_best_move || 0 == search_best_move->depth)
    {
        uci_debug("Didn't have a best move in bestmove() :-(");
        printf("bestmove 000\n");
        fflush(stdout);
    }
    else
    {
        char buf[16];
        move_to_string(&search_best_move->move, buf);
        uci_debug("giving out best move:");
        uci_debug(buf);
        printf("bestmove %s\n", buf);
        fflush(stdout);
    }
}

static void uci_free_stuff()
{
    if (uci_state)
    {
        uci_debug("Freeing uci_state");
        free(uci_state);
        uci_state = 0;
    }
}

void uci_new_game()
{
    uci_debug("Starting new game..");
    uci_debug("Wiping hash tables..");
    uci_free_stuff();
    hash_wipe();
}
 
void uci_init_position(char *position)
{
    uci_debug("Initializing a position:");
    uci_debug(position);

    if (uci_childpid)
    {
        uci_debug("Halting search in init_position");
        uci_halt_search();
    }

    uci_free_stuff();

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

        uci_debug("Unused part of newposition:");
        uci_debug(position);
    }
}

void uci_set_hash_size(int value)
{
    uci_debug("Setting hash table size..");
    // uci_debug("Setting hash table size to %dMB (%d entries)\n", value, tsize);
    int tsize = (value * 1024 * 1024) / sizeof(hash_node_t);
    hash_set_tsize(tsize);
}

void uci_debug(char *str)
{
    fprintf(uci_logfile, "DEBUG: %s\n", str);
    fflush(uci_logfile);
}

static void uci_sigquit(int n)
{
    n = 0;
    uci_debug("Got SIGQUIT");
    exit(66);
}

static void uci_sigusr1(int n)
{
    n = 0;
    uci_debug("Got SIGUSR1");
    uci_bestmove();
    exit(77);
}

void uci_go()
{
    assert(!uci_childpid);

    uci_debug("Forking for search_go()..");
    switch (uci_childpid = fork())
    {
        case -1:
            perror("fork error");
            uci_quit();
            break;
        case 0:
            signal(SIGQUIT, uci_sigquit);
            signal(SIGUSR1, uci_sigusr1);
            search_go(uci_state, 7);
            uci_debug("Child ended. Giving out my best move.");
            uci_bestmove();
            uci_childpid = 0;
            exit(55);
            break;
        default:
            break;
    }
}
