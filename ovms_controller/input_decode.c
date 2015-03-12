
#include "kb_manager.h"
#include "serial_link.h"
#include "tools.h"
#include "constants.h"
#include "routine.h"

#include <stdio.h>
#include <termios.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>


//////////////////////////////
// PROTOTYPES
//////////////////////////////

void input_decode(int *mode, char *buffer, char c);
void write_decode(int *mode, char *buffer, char c);
void read_decode(int *mode, char *buffer, char c);
int cmd_decode(int *mode, char c);


// kb_decode(mode, buffer): Interpret keyboard depending on the current mode
int kb_decode(int *mode, char *buffer)
{
	int isRunning = 1;
	char c = 0;
	
	c=get_key();
	
	if (c != 0)
	{
		switch (*mode)
		{
			case 0:
				isRunning = cmd_decode(mode, c);
				break;
			case 1:
				input_decode(mode, buffer, c);
				break;
			case 2:
				write_decode(mode, buffer, c);
				break;
			case 3:
				read_decode(mode, buffer, c);
				break;
			default:break;
		}
	}
	
	return isRunning;
}

//////////////////////////////
// PRIVATE
//////////////////////////////

// cmd_decode(mode, c): Interpret a pressed key as a command shortcut
int cmd_decode(int *mode, char c)
{
	static int on = 0;
	static int f_throttle = 0;
	static int r_throttle = 0;
	static int security = 0;
	//static int brake = 0;
	
	printf("\n");
	switch(c)
	{
		case 'h': 
			send("HELP\r");
			break;
		case 'z':
			reboot(); // send reboot signal and attempt diagnostic
			break;
		case 'b':
			bruteforce();
			break;
		case 'a':
			send("ATE\r");
			break;
		case 'i': // enter input mode
			*mode=1;
			printf("\n# Entering INPUT mode #\n");
			printf("- Press ESC to go back to CMD mode -\n");
			break;
		case 'w': //enter write mode
			*mode=2;
			printf("\n# Write command #\n[Index] [Subindex] [Value]\n");
			break;
		case 'r': //enter read mode
			*mode=3;
			printf("\n# Read command #\n[Index] [Subindex]\n");
			break;
		case 'q': // cleanly exit program
			return 0;
			break;
		case 'x': // enable/disable the most critical commands
			if (security == 0) 
			{
				security = 1;
				printf("Warning : Security disabled\n");
			}
			else
			{
				security = 0;
				printf("Security enabled\n");
			}
		default:break;
	}
			
	/*********************
	 * Critical commands *
	 *********************/
	 
	if (security == 1)
	{
		switch(c)
		{
			case '1': // remote on
				if (on == 0)
				{
					send("m 101,0,0,0,0,0,0,1\r"); //send("m 102,0,1,0,0\r");
					on = 1;
				}
				else
				{
					send("m 101,0,0,0,0,0,0,1\r"); //send("m 102,0,0,0,0\r");
					on = 0;
				}
				printf ("\n ## ## CASE 1 : REMOTE = %d\n", on) ;
				break;
			case '2': // fwd throttle
				if (f_throttle == 0)
				{
					send("m 101,20\r"); //send("m 101,20,0,0\r");
					f_throttle = 1;
					r_throttle = 0;
				}
				else
				{
					send("m 101,0\r"); //send("m 101,0,0,0,0\r");
					f_throttle = 0;
				}
				printf ("\n ## ## CASE 2 : FW THROTTLE  f = %d - r = %d\n", f_throttle, r_throttle) ;
				break;
			case '3': // rev throttle
				if (r_throttle == 0)
				{
					send("m 101,0,20\r"); //send("m 101,0,30,0,0\r");
					r_throttle = 1;
					f_throttle = 0;
				}
				else
				{
					send("m 101,0,0\r"); //send("m 101,0,0,0,0\r");
					r_throttle = 0;
				}
				printf ("\n ## ## CASE 3 : REV THROTTLE  f = %d - r = %d\n", f_throttle, r_throttle) ;
				break;
			case '4': // autobrake
				send("m 101,0,0,1,0\r"); //send("m 101,0,0,1,0\r");
				r_throttle = 0;
				f_throttle = 0;
				printf ("\n ## ## CASE 4 : AUTOBRAKE\n") ;
				break;
			case 'o': // fwd=1 back=0
				send("s cfg write 2121 0 1\rs cfg write 2122 0 0\r");
				printf ("\n ## ## << Forward >> \n") ;
				break;
			case 'p': // fwd=0 back=1
				send("s cfg write 2121 0 0\rs cfg write 2122 0 1\r");
				printf ("\n ## ## << Backward >> \n") ;
				break;
			case 'l': // fwd=back=0
				send("s cfg write 2121 0 0\rs cfg write 2122 0 0\r");
				printf ("\n ## ## << Neutral >> \n") ;
				break;
			case 'j': // override the throttle
				send("s cfg write 2910 4 16000\r");
				printf ("\n ## ## << THROTTLE Override >> \n") ;
				break;
			case 'k': // throttle back to normal
				send("s cfg write 2910 4 32769\r");
				printf ("\n ## ## << THROTTLE Back to Normal >> \n") ;
				break;
			case ',': // lock speed to 5
				send("s lock 5\r");
				printf ("\n ## ## << SPEED Lock 5km/h>> \n") ;
				break;
			case '.': // unlock speed
				send("s unlock\r");
				printf ("\n ## ## << SPEED Unlock >> \n") ;
				break;
			case '/': // keep alive
				send("m 101\r"); //or send("m 101,\r");
				printf ("\n ## ## << KEEP ALIVE >> \n") ;
				break;
			default:break;				
		}
	}
	else if (c=='o' || c=='p' || c=='l' || c=='j' || c==',' || c=='.'
		|| c=='k' || c=='1' || c=='2' || c=='3' || c=='4' || c=='/' )
	{
		printf("Unavailable command : Press 'x' to disable security\n");
	}
	return 1;
}


// input_decode(mode, buffer, c): Actions while in input mode
void input_decode(int *mode, char *buffer, char c)
{
	if (c == '\e') 
	{
		if (strlen(buffer) == 0)  // go back to command mode if ESC
		{
			cursor_left(2);
			*mode = 0;
			printf("  \n# Leaving INPUT mode #\n%s",CMD_LIST);
		}
		else
		{
			cursor_left(2);
			printf(">ABORT\n");
			memset(buffer, 0, 100);
		}
	}
	else if (c == '\n' || c == '\r') // send data to serial if RETURN
	{
		append(buffer, '\r');
		send(buffer);
		memset(buffer, 0, 100);
		printf("\n");
	}
	else // add to buffer
	{
		append(buffer, c);
	}
}

// read_decode(mode, buffer, c): Grabs keyboard input and appends read prefix
void read_decode(int *mode, char *buffer, char c)
{
	int i = 0, count = 0;
	
	if (c == '\e') 
	{
		if (strlen(buffer) == 0)  // go back to command mode if ESC
		{
			cursor_left(2);
			*mode = 0;
			printf("  \n# Read canceled #\n%s",CMD_LIST);
		}
		else
		{
			cursor_left(2);
			printf(">ABORT\n");
			memset(buffer, 0, 100);
		}
	}
	else
	{
		if (c == '\n' || c == '\r') // proceed writing if RETURN
		{
			count = 0;
			for (i=strlen(buffer) ; i >= 0 ; i--)
				if (buffer[i]==' ') count++; // count the number of spaces
			if (count == 1) // format is <X Y> 
			{
				send("s cfg read ");
				send(buffer);
				send("\r");
				//log2file("/root/dev/sevcon_changes.log", buffer);
				//*mode = 0; // leave mode
			}
			else
			{
				printf("Error: Wrong command format\n");
			}
			memset(buffer, 0, 100);
			printf("\n");
		}
		else // add to buffer
		{
			append(buffer, c);
		}
	}
}

// write_decode(mode, buffer, c): Parse command and initiate read/write
void write_decode(int *mode, char *buffer, char c)
{
	int i = 0, count = 0;
	char temp[100];
	
	if (c == '\e') 
	{
		if (strlen(buffer) == 0)  // go back to command mode if ESC
		{
			cursor_left(2);
			*mode = 0;
			printf("  \n# Write canceled #\n%s",CMD_LIST);
		}
		else
		{
			cursor_left(2);
			printf(">ABORT\n");
			memset(buffer, 0, 100);
		}
	}
	else
	{
		if (c == '\n' || c == '\r') // proceed writing if RETURN
		{
			strcpy(temp,buffer);
			count = 0;
			// parse values of index and subindex
			for (i=strlen(temp) ; i >= 0 , temp[i]!=' ' ; i--);
			temp[i]=0;
			for (i=strlen(temp) ; i >= 0 ; i--)
				if (temp[i]==' ') count++; // count the number of spaces
			if (count == 1) // format is <X Y> 
			{
				send("s cfg read ");
				send(temp);
				send("\r");
				log2file("/root/dev/sevcon_changes.log", buffer);
				set_command(buffer);
				set_routine(3);
				//*mode = 0; // leave mode
			}
			else
			{
				printf("Error: Wrong command format\n");
			}
			memset(buffer, 0, 100);
			printf("\n");
		}
		else // add to buffer
		{
			append(buffer, c);
		}
	}
}
