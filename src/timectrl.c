#include <sys/time.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <stdio.h>
#include "timectrl.h"
#include "search.h"
#include "uci.h"
#include "util.h"

timecontrol_t timecontrol;

void timectrl_go(state_t *state, int wtime, int btime, int ponder, int depth, int nodes, int infinite, int verbose)
{
    memset(&timecontrol, 0, sizeof(timecontrol_t));

    if (!depth)
    {
        depth = 8;
    }

    timecontrol.wtime = wtime;
    timecontrol.btime = btime;
    timecontrol.ponder = ponder;
    timecontrol.depth = depth;
    timecontrol.nodes = nodes;
    timecontrol.infinite = infinite;
    timecontrol.verbose = verbose;

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

    if (timecontrol.verbose)
    {
        struct timeval now;
        gettimeofday(&now, NULL);

        double spent_time;
        spent_time = now.tv_sec - timecontrol.start_time.tv_sec;
        spent_time *= 1000000;
        spent_time += (now.tv_usec - timecontrol.start_time.tv_usec);
        spent_time /= 1000000.0;

        printf("info nodes %llu nps %d",
            search_data.visited_nodes, (int)(search_data.visited_nodes / spent_time));

        if (search_data.pv[0].depth > 0)
        {
            printf(" depth %d score cp %.2f time %d pv", search_data.pv[0].depth, (float)search_data.pv[0].score / 100, (int)spent_time * 1000);

            int i;
            for (i = 0; i < 128; ++i)
            {
                char buf[16];
                if (!search_data.pv[i].depth)
                {
                    break;
                }

                util_move_to_lan(search_data.pv[i].move, buf);
                printf(" %s", buf);
            }
        }

        printf(" cachehits %d cachemisses %d", search_data.cache_hits, search_data.cache_misses);

        printf("\n");
        fflush(stdout);
    }

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
