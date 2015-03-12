#ifndef SERIAL_LINK_H
#define SERIAL_LINK_H

int open_port();
int close_port();
void send(char* msg);
void next_line();
int init_serial(int i);
char* get_line();
void clear_buffer();

#endif
