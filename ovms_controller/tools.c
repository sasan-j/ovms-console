
#include "tools.h"

#include <stdio.h>
//#include <unistd.h>
#include <string.h>
#include <time.h>

// append(s,c): Contatenate char c to string s
void append(char* s, char c)
{
        int len = strlen(s);
        s[len] = c;
        s[len+1] = '\0';
}

// elapsed _time(t): Calculate elapsed time since t (in seconds)
int elapsed_time(time_t t)
{
	return (int) difftime(time(0),t);
}

// contains(string, match): Return true if 'string' contains 'match'
int contains(char *string, char *match)
{
	return (strstr(string,match) != NULL);
}

// cursor_left(n): Shift the cursor to the left N times
void cursor_left(int n)
{
	printf("\33[%dD",n);
}

// spy(msg, N): Display a debug message to top right corner with specific offset
void snitch(char* msg, int N)
{
	printf("\033[s"); // save cursor position
	printf("\033[0;70H"); // put cursor to top right corner
	printf("\033[%dB", N); // go down N lines
	printf("%s", msg);
	printf("\033[u"); // restore initial cursor position
	fflush( stdout );
}


void log2file(char* file_name, char * text)
{
	FILE *ofp;
	time_t now = time(NULL);
	struct tm * timeinfo = localtime(&now);
	char date[100];
    strftime(date, sizeof date, "%Y-%m-%d %H:%M:%S", timeinfo);
	
	fflush( stdout );
	ofp = fopen(file_name,"a");
	if (ofp == NULL)
	{
		fprintf(stderr, "Can't open file: %s\n",file_name);
	}
	else
	{
		//printf("[logged] : %s\n", text);
		fprintf(ofp, "%s : %s\n", date,  text);
		fclose(ofp);
	}
	fflush(stdout);
}

