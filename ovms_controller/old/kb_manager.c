#include <stdio.h>
#include <termios.h>
#include <unistd.h>
#include <fcntl.h>

#include "kb_manager.h"
#include "serial_link.h"

int kb_decode(int *mode)
{
	char c = 0;
	char s[2]={'0','\0'};
	if (kbhit()) // if a key has been pressed
	{
		c = getc(stdin); // get which key has been pressed
		
		if (*mode == 0) // if in command mode
		{
			switch(c){
				case 'h': 
					send("help\r");
					break;
				case 'r':
					send("reset\r");
					break;
				case 'a':
					send("ate\r");
					break;
				case 's':
					send("setup\r");
					break;
				case 'i': // enter input mode
					*mode=1;
					printf("\n# Entering INPUT mode #\n");
					break;
				case 'q': // cleanly exit program
					return 0;
					break;
				default:break;				
			}
		}
		else
		{
			s[0] = c;
			if (c == '\e')
			{
				*mode = 0; // go back to command mode
				printf("\n# Leaving INPUT mode #\n");
			}
			else
				send(s); // pass the input character to the serial
		}
	}
	return 1;
}

int kbhit(void)
{
	struct termios oldt, newt;
	int ch;
	int oldf;

	tcgetattr(STDIN_FILENO, &oldt);
	newt = oldt;
	newt.c_lflag &= ~(ICANON | ECHO);
	tcsetattr(STDIN_FILENO, TCSANOW, &newt);
	oldf = fcntl(STDIN_FILENO, F_GETFL, 0);
	fcntl(STDIN_FILENO, F_SETFL, oldf | O_NONBLOCK);

	ch = getchar();

	tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
	fcntl(STDIN_FILENO, F_SETFL, oldf);

	if(ch != EOF)
	{
		ungetc(ch, stdin);
		return 1;
	}

	return 0;
}
