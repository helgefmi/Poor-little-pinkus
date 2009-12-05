#include <sys/time.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <stdio.h>
#include "timectrl.h"
#include "search.h"
#include "plp.h"
#include "uci.h"
#include "util.h"

timecontrol_t timecontrol;

void timectrl_go(state_t *state, int wtime, int btime, int ponder, int depth, uint64_t nodes, int infinite, int verbose)
{
    int mytime = (state->turn == WHITE ? wtime : btime) / 1000;

    memset(&timecontrol, 0, sizeof(timecontrol_t));

    if (!depth || infinite)
        depth = 200;

    timecontrol.state = state;
    timecontrol.verbose = verbose;
    timecontrol.nodes = nodes;
    timecontrol.depth = depth;

    timecontrol.search_time_left = (mytime ? mytime / 25 : 999999999);
    timecontrol.ponder = ponder;
    timecontrol.wtime = wtime;
    timecontrol.btime = btime;
    timecontrol.infinite = infinite;
    timecontrol.input_timer = INPUT_INTERVAL;

    signal(SIGALRM, timectrl_alarm);

    gettimeofday(&timecontrol.start_time, NULL);
    timecontrol.searching = 1;
    alarm(1);

    search_go(state, depth);

    timecontrol.searching = 0;
}

void timectrl_alarm(int n)
{
    if (!timecontrol.searching)
    {
        return;
    }

    n = 0;

    if (timecontrol.search_time_left)
    {
        --timecontrol.search_time_left;
    }

    if (timecontrol.verbose)
    {
        timectrl_notify_uci(timecontrol.state);
    }

    alarm(1);
}

int timectrl_should_halt()
{
    fd_set rdfs;
    struct timeval tv;
    int ret;

    if (timecontrol.input_timer-- == 0)
    {
        timecontrol.input_timer = INPUT_INTERVAL;
        FD_ZERO(&rdfs);
        FD_SET(0, &rdfs);

        tv.tv_sec = 0;
        tv.tv_usec = 0;

        ret = select(1, &rdfs, NULL, NULL, &tv);

        if (ret == -1)
        {
        }
        else if (ret)
        {
            uci_input();
        }
    }

    if (!timecontrol.searching)
    {
        return 1;
    }
    if (timecontrol.nodes && search.visited_nodes > timecontrol.nodes)
    {
        return 1;
    }

    if (timecontrol.search_time_left <= 0)
    {
        return 1;
    }

    return 0;
}

void timectrl_notify_uci()
{
    struct timeval now;
    gettimeofday(&now, NULL);

    double spent_time;
    spent_time = now.tv_sec - timecontrol.start_time.tv_sec;
    spent_time *= 1000000;
    spent_time += (now.tv_usec - timecontrol.start_time.tv_usec);
    spent_time /= 1000000.0;

    printf("info nodes %llu nps %d depth %d cachehits %d cachemisses %d time %d",
        search.visited_nodes,
        (int)(search.visited_nodes / spent_time),
        search.max_depth,
        search.cache_hits,
        search.cache_misses,
        (int)spent_time * 1000);

    util_print_pv();

    printf("\n");
    fflush(stdout);
}
