#include "serial_link.h"
#include "tools.h"

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <termios.h>
//#include <unistd.h>
#include <string.h>
#include <fcntl.h>

//////////////////////////////
// PROTOTYPES
//////////////////////////////

typedef struct node node;

struct node {
  char line[100];
  struct node *next;
};

node * lineBuffer;
char charBuffer[100];
int fd;
char addr[20]="/dev/ovms";

int set_interface_attribs (int fd, int speed);
int decapitate(node ** head);
void add_line(node ** head, char newLine[100]);
void set_port(int i);


//////////////////////////////
// PUBLIC
//////////////////////////////


// init_serial(i): Initialize serial link with port 'i'
int init_serial(int i)
{
	if (i != -1)
		set_port(i);
	fd = 0;	// File descriptor
	lineBuffer = malloc(sizeof(node));
	strcpy(lineBuffer->line,"");
	lineBuffer->next = NULL;
	return open_port();
}

// next_line(): Read, print and register the next line \
				from the serial link
void next_line()
{
	int i, pos;
    char input [200];
    
    ssize_t length = read(fd, &input, sizeof(input));
	if (length > 0)
    {
        input[length] = '\0';
        pos = strlen(charBuffer); // go to the last character: '\0'
        
        fflush( stdout );
        printf("%s",input);
        fflush( stdout );
		// for the number of input characters
        for( i = 0; i < length; i++)
		{
			if (pos < 99)
			{
				charBuffer[pos] = input[i];
				charBuffer[pos+1] = '\0'; // just in case
				
				// display in tty
				//printf("%c",charBuffer[pos]);
				
				pos++;
				
				// if it's a new line, add to the line buffer
				if (input[i] == '\r' && input[i+1] == '\n')
				{
					add_line(&lineBuffer, charBuffer);
					strcpy(charBuffer,"");
					pos = 0;
				}
			}
			else // if the char buffer is full
			{
				// dump to the line buffer to avoid char buffer overflow
				add_line(&lineBuffer, charBuffer);
				strcpy(charBuffer,"");
				//memset(charBuffer, 0, 100);
				pos = 0;
				charBuffer[pos] = input[i];
				charBuffer[pos+1] = '\0';
			}
		}
    }
}

// get_line(): Get the last line read
char* get_line()
{
	char* line;
	line = malloc(100 * sizeof(char));
	if (lineBuffer != NULL)
	{
		strcpy(line,lineBuffer->line);
		decapitate(&lineBuffer);
	}
	else
	{
		strcpy(line,"");
	}
	
	return line;
}

void clear_buffer()
{
	node * next_node = NULL;
	while (lineBuffer != NULL)
	{
		next_node = lineBuffer->next;
		free(lineBuffer);
		lineBuffer = next_node;
	}
}

// send(msg): Send a string of characters to the serial link
void send(char *msg)
{
	if (write(fd, msg, strlen(msg)) < 0 )
		printf("Failed to write in port\n");
	//else
		//printf("\n> %s\n", msg);
}

// close_port(): Close tty port
int close_port(void)
{
	return close(fd);
}

// open_port(): Open tty port and set file descriptor for ttyUSB
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
		// set speed to 9,600 bps
		if (set_interface_attribs (fd, B9600) == 0) 
			printf("Port configured\n");
		else
			printf("Port configuration failed\n");
	}

	return (fd);
	
}

//////////////////////////////
// PRIVATE
//////////////////////////////

void set_port(int i)
{
	char temp [20];
	sprintf(temp, "/dev/ttyUSB%d", i);
	strcpy(addr, temp);
}

// decapitate(head): Remove the head of a linked list 
int decapitate(node ** head)
{
	node * temp = *head;
	node * next_node = NULL;

	if (temp == NULL) {
		return -1;
	}

	next_node = temp->next;
	free(temp);
	temp = next_node;
	*head = temp;
	return 0;
}

// add_line(head, newLine): Add a new node to a linked list with \
							newLine as char* value
void add_line(node ** head, char* newLine)
{
	node * current = * head;
	node * temp;
	temp = malloc(sizeof(node));
	strcpy(temp->line, newLine);
	temp->next = NULL;
	
	if (current == NULL)
	{
		*head = temp;
	}
	else
	{
		while (current->next != NULL)
			current = current->next;
		
		current->next = temp;
	}
}

// set_interface_attribs(fd, speed): Write tty configuration
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
        
        // Flush the port's buffers (in and out) before we start using it
        tcflush(fd, TCIOFLUSH);

        cfsetospeed (&options, speed);
        cfsetispeed (&options, speed);
		
        options.c_cflag &= ~CSIZE;
        options.c_cflag |= ~CS8;		// 8-bit chars
        options.c_cflag &= ~PARENB;		// N (no parity)
        options.c_cflag &= ~CSTOPB;		// 1 stop bit
		options.c_cflag &= ~CRTSCTS;	// Disable hardware flow control
		
		// INPUT
		// Disable software flow control
        options.c_iflag &= ~(IXON | IXOFF | IXANY);	
		
		// OUTPUT
		options.c_oflag = 0;
		
		// LOCAL
		
		options.c_lflag |= ~(ICANON);// enable canonical (line by line)
		options.c_lflag &= ~(ECHO | ECHOE); // no local echo
		
		tcflush(fd, TCIOFLUSH);
		
        if (tcsetattr (fd, TCSANOW, &options) != 0)
        {
                fprintf(stderr, "error from tcsetattr - %s",
			 strerror(errno));
                return -1;
        }
        
        return 0;
}
