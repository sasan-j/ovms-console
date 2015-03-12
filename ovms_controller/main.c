
#include <stdio.h>   /* Standard input/output definitions */
//#include <stdlib.h>
#include <unistd.h>  /* UNIX standard function definitions */
#include <errno.h>   /* Error number definitions */
//#include <string.h>  /* String function definitions */

#include "serial_link.h"
//#include "kb_manager.h"
#include "input_decode.h"
#include "constants.h"


int main()
{
	// --------------------------
	// INITIALISATION
	// --------------------------
	
	// VARIABLES
	
	int mode = 0; // 0: command; 1: input 
	int isRunning = 1; 
	char buffer[100] = "";
	int portNb = -1;
	
	
    printf( "OVMS Controller: Build 0.8\n" );
    
    // SERIAL PORT
    
    while (init_serial(portNb) == -1)
    {
		printf("Enter ttyUSB port number : ");
		if (scanf("%d", &portNb) != 1) printf("Incorrect port number\n");
	}
	
	// --------------------------
	// MAIN LOOP
   	// --------------------------
   	
	printf("# Entering COMMAND mode #\n%s",CMD_LIST);
	
	while(isRunning) // stops loop if 'q' has been issued
	{
		usleep(10000); // don't overload the system
		next_line(); // read next line from the serial		
		isRunning = kb_decode(&mode, buffer); // read keyboard
		do_routine(); // routine process
	}
	
	// --------------------------
    // EXITING
   	// --------------------------
   	
    // close the serial port before exiting
    close_port();
    return 0;
}
