#include "sort.h"
#include "move.h"

void sort_moves(move_t *moves, int count)
{
    int sort_values[100];
    int i;
    for (i = 0; i < count; ++i)
    {
        sort_values[i] = moves[i].capture - moves[i].piece;
    }
}
