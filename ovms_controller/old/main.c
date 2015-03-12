
#include <stdio.h>   /* Standard input/output definitions */
#include <stdlib.h>
#include <unistd.h>  /* UNIX standard function definitions */
#include <errno.h>   /* Error number definitions */
//#include <string.h>  /* String function definitions */

#include "serial_link.h"
#include "kb_manager.h"


int main()
{
	int mode = 0; // 0: command; 1: input 
	int isRunning = 1; 
    printf( "Build 0.4\n" );
    
    open_port();
    
	printf("# Entering COMMAND mode #:\n\th:help r:reset a:ate s:setup\n");
	
	while(isRunning) // stops loop if 'q' has been issued
	{
		usleep(10000); // don't overload the system
		next_line(); // read next line from the serial		
		isRunning = kb_decode(&mode); // read keyboard
	}
    
    // close the serial port before exiting
    close_port();
    return 0;
}
