#ifndef _XBOARD_H
#define _XBOARD_H

void uci_start();

void uci_input();
void uci_parse_cmd(char*);

void uci_quit();
void uci_halt_search();
void uci_new_game();
void uci_init_position(char*);
void uci_set_hash_size(int);
void uci_go();

void uci_debug(char*);
#endif
