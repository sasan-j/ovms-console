#include "kb_manager.h"
#include "serial_link.h"

#include <stdio.h>
#include <termios.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>

//////////////////////////////
// PROTOTYPES
//////////////////////////////

int kbhit(void);


//////////////////////////////
// PUBLIC
//////////////////////////////

// get_key(): Non blocking getchar
char get_key()
{
	char c = 0;
	if (kbhit()) // if a key has been pressed
	{
		c = getc(stdin); // get which key has been pressed
	}
	return c;
}

//////////////////////////////
// PRIVATE
//////////////////////////////

// kbhit(): Return 1 when a key is pressed
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
