#ifndef SERIAL_CONF
#define SERIAL_CONF

int set_interface_attribs (int fd, int speed);

int open_port(void);

int close_port(void);

void send(char* msg);

void next_line();

#endif
