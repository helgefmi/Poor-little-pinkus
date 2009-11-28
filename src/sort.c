#include "sort.h"
#include "move.h"
#include "plp.h"

void sort_moves(int *moves, int count)
{
    int sort_values[100];
    int i;
    for (i = 0; i < count; ++i)
    {
        sort_values[i] = MoveCapture(moves[i]) - MovePiece(moves[i]);
    }
}
