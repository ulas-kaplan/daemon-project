#include "log.h"                            // Includes the corresponding header file.

#include <syslog.h>                         // Includes definitions for system logging.
#include <stdio.h>                          // Includes standard I/O operations.

int log_level = 1;                          // Initializes the default logging level to 1.
int log_to_syslog = 0;                      // Initializes the syslog flag to disabled.
FILE *log_file = NULL;                      // Initializes the log file pointer to null.

void do_log(int level, const char *msg)     // Implements the logging function.
{                                           // Begins the function body.
	int sys_lvl = LOG_INFO;             // Sets the default syslog level to info.

	if (level > log_level)              // Checks if the message level exceeds the current log level.
		return;                     // Exits the function if the log level is too high.

	if (log_to_syslog) {                // Checks if syslog is enabled.
		if (level == 0)             // Checks if the message is an error.
			sys_lvl = LOG_ERR;  // Sets the syslog level to error.
		else if (level == 2)        // Checks if the message is a debug log.
			sys_lvl = LOG_DEBUG;// Sets the syslog level to debug.
		
		syslog(sys_lvl, "%s", msg); // Writes the message to the system log.
	} else if (log_file) {              // Checks if a file log is configured.
		fprintf(log_file, "%s", msg); // Writes the message to the designated log file.
		fflush(log_file);           // Flushes the output buffer to ensure immediate writing.
	}                                   // Closes the else-if block.
}                                           // Ends the function body.