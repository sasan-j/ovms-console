
#include <stdio.h>
#include <errno.h>
#include <termios.h>
#include <unistd.h>
#include <string.h> /* memset */
#include <fcntl.h>
#include "serial_link.h"

// -------------------------
// Global variables

// File descriptor of the port
int fd=0;
// Port address
char* addr="/dev/ttyUSB5";


// -------------------------
// Functions

// read next line from serial
void next_line()
{
    char buffer[100];
    ssize_t length = read(fd, &buffer, sizeof(buffer));
	if (length > 0)
    {
        buffer[length] = '\0';
        printf("%s", buffer);
    }
}

// write data to serial
void send(char* msg)
{
	
	if ( write(fd, msg, sizeof(msg)) < 0 )
		puts("write() failed\n");
	//else
		//printf("> %s\n", msg);
}

// close tty
int close_port(void)
{
	return close(fd);
}

// open tty
int open_port(void)
{
	printf("Opening port %s\n",addr);

	fd = open(addr, O_RDWR | O_NOCTTY | O_NDELAY);
	if (fd == -1)
	{
		// Could not open the port.
		fprintf(stderr, "open_port: Unable to open %s - %s\n", addr,
			 strerror(errno));
	} 
	else
	{
		printf("Port successfully opened\n");
		if (set_interface_attribs (fd, B9600) == 0)  // set speed to 9,600 bps
			printf("Port configured\n");
	}

	return (fd);
	
}

// configure tty settings
int set_interface_attribs (int fd, int speed)
{
        struct termios options;
        memset (&options, 0, sizeof options);
        if (tcgetattr (fd, &options) != 0)
        {
                fprintf(stderr, "error from tcgetattr - %s",
			 strerror(errno));
                return -1;
        }
        
        cfmakeraw(&options);
        
        // Flush the port's buffers (in and out) before we start using it
        tcflush(fd, TCIOFLUSH);

        cfsetospeed (&options, speed);
        cfsetispeed (&options, speed);

        options.c_cflag = (options.c_cflag & ~CSIZE) | CS8;		// 8-bit chars
        options.c_cflag &= ~PARENB;		// N (no parity)
        options.c_cflag &= ~CSTOPB;		// 1 stop bit

		// Disable hardware flow control
		options.c_cflag &= ~CRTSCTS;	
        // Disable software flow control
        options.c_iflag &= ~(IXON | IXOFF | IXANY); 

        //options.c_cc[VMIN]  = 0;		// read doesn't block
        //options.c_cc[VTIME] = 5;		// 0.5 seconds read timeout
		
		//options.c_lflag &= ~(ICANON | ECHO | ISIG)		// enable raw input
		options.c_lflag |= ~(ICANON/* | ECHO | ECHOE*/);		// enable canonical (line by line)
		options.c_lflag &= ~(ECHO | ECHOE);
		
        if (tcsetattr (fd, TCSANOW, &options) != 0)
        {
                fprintf(stderr, "error from tcsetattr - %s",
			 strerror(errno));
                return -1;
        }
        
        return 0;
}


/*
usleep ((7 + 25) * 100);             // sleep enough to transmit the 7 plus
                                     // receive 25:  approx 100 uS per char transmit
