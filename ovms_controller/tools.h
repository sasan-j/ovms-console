#ifndef TOOLS_H
#define TOOLS_H

#include <time.h>


void append(char* s, char c);
int elapsed_time(time_t t);
int contains(char *request, char *match);
void cursor_left(int n);
void snitch(char* msg, int N);
void log2file(char* file_name, char * text);


#endif
