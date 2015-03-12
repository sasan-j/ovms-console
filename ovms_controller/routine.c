#include "routine.h"
#include "serial_link.h"
#include "tools.h"
#include "constants.h"

#include <stdio.h>
#include <stdlib.h>
//#include <unistd.h>
#include <string.h>
#include <time.h>


// PROTOTYPES
void r_write();
void r_diagnostic();
void r_bruteforcing();
void end_routine();

int currentRoutine = 0;
char write_cmd [100];

//////////////////////////////
// PUBLIC
//////////////////////////////

// set_command(s): Set command value
void set_command(char* s)
{
	strcpy(write_cmd, s);
}

// set_routine(): Switch to designated routine
void set_routine(int routine)
{
	if (currentRoutine == 0 && routine > 0 && routine < 4)
	{
		currentRoutine = routine;
	}
	else if (currentRoutine != 0 )
		fprintf(stderr, "Denied: %s is running\n", ROUTINE_NAME[currentRoutine]);
	else
		fprintf(stderr, "Denied: routine %d is not implemented\n", routine);
}

// do_routine(): Follow the current routine's instuctions
void do_routine()
{
	switch (currentRoutine)
	{
		case 1:
			r_diagnostic();
			break;
		case 2:
			r_bruteforcing();
			break;
		case 3:
			r_write();
			break;
		default:
			break;
	}
}

// get_routine(): Return current routine
int get_routine()
{
	return currentRoutine;
}

// reboot(): herp derp
// TODO: maybe put in an other file more related to advanced commands
void reboot()
{
	send("s reset\r");
	set_routine(1);
}

// bruteforce(): Start the bruteforce routine
void bruteforce()
{
	set_routine(2);
}

//////////////////////////////
// PRIVATE
//////////////////////////////

void r_write()
{
	static int state = 0;
	static time_t t1;
	
	char* line;

	switch (state)
	{
		case 0:
			t1=time(0);
			state = 1;
			clear_buffer();
			break;
		case 1: // wait for read response & log
		
			line = get_line();
			
			if (elapsed_time(t1) >= 2) /*abort if timeout*/
			{
				printf("Failed to retrieve response\n");
				log2file("/root/dev/sevcon_changes.log", "Failed to parse read ACK");
				state = 3;
			}
			if (contains(line,"CFG READ:  SDO")) /*contains normal read response*/
			{
				line++;
				log2file("/root/dev/sevcon_changes.log", line);
				append(write_cmd, '\r');
				send("s cfg write ");
				send(write_cmd);
				state = 2;
				t1=time(0);
				// proceed with write
			}
			else if (contains(line,"CFG READ: ERROR")) /*contains read error*/
			{
				if (contains(line,"0x06010001")) /*write only*/
				{
					log2file("/root/dev/sevcon_changes.log", "Object is write only");
					append(write_cmd, '\r');
					send("s cfg writeo ");
					send(write_cmd);
					state = 2;
					t1=time(0);
				}
				else
				{
					log2file("/root/dev/sevcon_changes.log", "Cannot read value, abort");
					state = 3;
				}
			}
			break;
		case 2: // wait for answer to write & log then back to 0
			line = get_line();
			
			if (contains(line,"CFG WRITEO: OK") || contains(line,"CFG WRITE: OLD"))
			{
				log2file("/root/dev/sevcon_changes.log", "Write OK");
				state = 3;
			}
			if (elapsed_time(t1) >= 2)
			{
				log2file("/root/dev/sevcon_changes.log", "Write FAIL");
				printf("Failed to prase write ACK\n");
				state = 3;
			}
			break;
		case 3:
			log2file("/root/dev/sevcon_changes.log", "*************\n");
			end_routine();
			state = 0;
			printf("\n");
			break;
	}
}

// r_diagnostic(): Routine for entering in diagnostic mode
void r_diagnostic()
{
	static time_t t1;
	static int timeout=10;
	static int diag_state = 0; // memorizes the current state of the routine

	char* line;
	
	line = get_line();
	
	/* State machine: 
	 * 0 - First launch, get rid of old inputs 
	 * 1 - waiting for 'Ready' signaling that OVMS has booted, 
	 * 2 - Waiting for diagnostic mode confirmation / sending ate-setup
	 * */
	switch(diag_state)
	{
		case 0:
			clear_buffer();
			printf("Waiting for OVMS to boot\n");
			diag_state = 1;
			t1=time(0);
			break;
		case 1: // Waiting for boot
			if (elapsed_time(t1) >= 2) // After 2 seconds have passed
			{
				send("SETUP\r");
				t1 = time(0);
			}
			if (contains(line,"GPS Ready") || contains(line,"Call Ready"))
			{
				printf("OVMS boot detected\n");
				// Send commands "setup" and "ate"
				send("ATE\r");
				diag_state = 2;
				t1 = time(0);
			} 
			if (contains(line,"# OVMS"))
			{
				diag_state = 3;
			}
			break;
		case 2: // Waiting for OVMS diagnostic mode
			if (elapsed_time(t1) >= 1) // After 1 second has passed
			{
				send("SETUP\r");
				t1 = time(0);
				timeout--;
			}
			if (timeout <= 0) // Failed if more that 10 sec passed
			{
				printf("DIAGNOSTIC TIMEOUT\n");
				diag_state = 3;
			}
			if (contains(line,"# OVMS")) // Success if '# OVMS' is read
			{
				diag_state = 3;
			}
			break;
		case 3:
			diag_state = 0;
			timeout=10;
			end_routine();
			printf("DIAG DONE\n");
			break;
		default:break;
	}
}

void r_bruteforcing()
{
	static time_t t1;
	static int try = 0;
	static int bf_state = 0;
	static uint pwd_value;
	char* line;
	char* file_name = "/root/dev/bruteforce.log";
	int start_value, end_value;
	char txt[50], cmd[50];
	int timeout = 3;
	
	start_value = 50300; //0
	end_value = 65535; //65535
	
	line = get_line();
	
	/* State machine: 
	 * 0 - First launch
	 * */
	switch(bf_state)
	{
		case 0:
			clear_buffer();
			printf("Bruteforce from %d to %d\n\n", start_value, end_value);
			bf_state = 1;
			pwd_value = start_value;
			break;
		case 1:
			send("S CFG WRITE 5000 3 0\r"); // set ID to 0
			bf_state = 2;
			t1 = time(0);
			break;
		case 2:
			if (contains(line,"CFG WRITE: OLD")) // parse answer from CAN
			{
				bf_state = 3;
				try = 0;
			}
			if (elapsed_time(t1) >= timeout) // timeout
			{
				fflush( stdout );
				printf("Failed to write ID\n");
				bf_state = 7;
			}
			break;
			
			// TODO wait for echo of command
			
		case 3: // send login command
			fflush( stdout );
			printf("PWD Value = %d\n", pwd_value);
			sprintf(cmd,"S CFG WRITEO 5000 2 %d\r", pwd_value);
			send(cmd);
			t1 = time(0);
			bf_state = 4;
			break;
		case 4: // wait for answer to login command
			if (contains(line,"CFG WRITEO: OK")) // parse answer from CAN
			{
				bf_state = 5;
				try = 0;
				sprintf(txt, "Login success with pwd = %d\n", pwd_value);
				log2file(file_name,txt);
			}
			else if (contains(line,"CFG WRITEO: ERROR 0008")) 
			{
				bf_state = 7;
				fflush( stdout );
				printf("Login failed with pwd = %d\n", pwd_value);
			} 
			else if (elapsed_time(t1) >= timeout) //
			{
				if (try > 2)
				{	
					sprintf(txt, "Failed to get answer for pwd = %d\n", pwd_value);
					log2file(file_name,txt);
					bf_state = 7;
				}
				else
				{
					fflush( stdout );
					printf("Retry login\n");
					try++;
					bf_state = 3;
				}
			}
			break;
		case 5: // verify which authentication level we are in
			send("S CFG READ 5000 1\r");
			t1 = time(0);
			bf_state = 6;
			break;
		case 6: // wait for answer of authentication level
			if (contains(line,"CFG READ:  SDO 0x5000.01")) // parse answer from CAN
			{
				sprintf(txt, "Authentication level = %c\n", line[(strlen(line)-2)]);
				log2file(file_name,txt);
				bf_state = 7;
			}
			else if (elapsed_time(t1) >= timeout) // timeout after 3 seconds
			{
				if (try > 2)
				{
					fflush( stdout );
					printf("Failed to parse authentication level\n");
					bf_state = 7;
				}
				else
				{
					try++;
					bf_state = 5;
				}
			}
			break;
		case 7:
			if (pwd_value < end_value)
			{
				pwd_value++;
				bf_state = 3;
				try = 0;
				fflush( stdout );
				printf("\n-------------------------\n");
				fflush( stdout );
			}
			else
			{
				bf_state = 0;
				try = 0;
				end_routine();
			}
			break;
		default:break;
	}
}

// end_routine(): Terminate current active routine
void end_routine()
{
	currentRoutine = 0;
}
