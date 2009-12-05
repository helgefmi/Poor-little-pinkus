#ifndef _UTIL_H
#define _UTIL_H

/* A collection of helper-functions for the moggio library. */

#include <stdint.h>
#include "state.h"
#include "move.h"

int util_char_to_piece(char);
int util_char_to_color(char);
int util_chars_to_square(char*);
void util_square_to_chars(uint64_t, char*);
char util_piece_to_char(int, int);
void util_int_to_bitmap(uint64_t);
int util_chars_to_move(char*, state_t*);
void util_move_to_lan(int, char*);
int util_legal_killer(state_t*, int);

char *util_trim_str(char*);

void util_print_pv();

#endif
