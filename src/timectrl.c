#include <sys/time.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <stdio.h>
#include "timectrl.h"
#include "search.h"
#include "uci.h"

timecontrol_t timecontrol;

void timectrl_go(state_t *state, int wtime, int btime, int ponder, int depth, int nodes, int infinite)
{
    memset(&timecontrol, 0, sizeof(timecontrol_t));

    if (!depth)
    {
        depth = 7;
    }

    timecontrol.wtime = wtime;
    timecontrol.btime = btime;
    timecontrol.ponder = ponder;
    timecontrol.depth = depth;
    timecontrol.nodes = nodes;
    timecontrol.infinite = infinite;

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
    --timecontrol.search_time_left;

    struct timeval now;
    gettimeofday(&now, NULL);

    double spent_time;
    spent_time = now.tv_sec - timecontrol.start_time.tv_sec;
    spent_time *= 1000000;
    spent_time += (now.tv_usec - timecontrol.start_time.tv_usec);
    spent_time /= 1000000.0;

    char buf[16];
    move_to_string(&search_data.move, buf);

    printf("info depth %d nodes %llu score %d nps %d",
        search_data.depth, search_data.nodes, search_data.score, (int)(search_data.nodes / spent_time));

    if (search_data.depth > 0)
    {
        printf(" time %d pv %s", (int)spent_time * 1000, buf);
    }

    printf("\n");
    fflush(stdout);

    alarm(1);
}

int timectrl_should_halt()
{
    if (timecontrol.searching)
    {
        return 1;
    }

    if (timecontrol.search_time_left <= 0)
    {
        return 1;
    }

    return 0;
}
