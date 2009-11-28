#include <stdio.h>
#include "sort.h"
#include "move.h"
#include "plp.h"
#include "util.h"

void sort_moves(int *moves, int count)
{
    int sort_values[100];
    int i;

    for (i = 0; i < count; ++i)
    {
        if (MoveCapture(moves[i]) < 6)
        {
            sort_values[i] = 512 + (MoveCapture(moves[i]) - MovePiece(moves[i]));
        }
        else
        {
            sort_values[i] = -MovePiece(moves[i]);
        }
    }

    register int swapped, tmp;
    register int *end = moves + count - 1;
    register int *move, *sortv;

    do
    {
        swapped = 0;
        for (move = moves, sortv = sort_values; move < end; ++move, ++sortv)
        {
            if (*sortv < *(sortv + 1))
            {
                tmp = *sortv;
                *sortv = *(sortv + 1); 
                *(sortv + 1) = tmp;
                tmp = *move;
                *move = *(move + 1); 
                *(move + 1) = tmp;
                swapped = 1;
            }   
        }
    } while (swapped);
}
