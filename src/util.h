#ifndef _UTIL_H
#define _UTIL_H

/* A collection of helper-functions for the moggio library. */

#include <stdint.h>

int util_char_to_piece(char);
int util_char_to_color(char);
uint64_t util_chars_to_square(char*);
void util_square_to_chars(uint64_t, char*);
char util_piece_to_char(int, int);
void util_int_to_bitmap(uint64_t);

#endif
