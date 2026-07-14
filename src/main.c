#define _XOPEN_SOURCE 700                   // Exposes POSIX standards for the main translation unit.

#include "daemon.h"                         // Includes the daemon functions and flags.
#include "log.h"                            // Includes the logging functions and variables.

#include <stdio.h>                          // Includes standard I/O for printing.
#include <stdlib.h>                         // Includes standard library utilities.
#include <string.h>                         // Includes string manipulation functions like strcmp.
#include <syslog.h>                         // Includes syslog definitions.
#include <getopt.h>                         // Includes command-line argument parsing.
#include <unistd.h>                         // Includes POSIX OS APIs like sleep.

int main(int argc, char *argv[])            // Defines the main entry point of the program.
{                                           // Begins the function body.
	int opt;                            // Declares a variable to store parsed options.

	/* -o ve -l argumanlarini ayristir */ 
	while ((opt = getopt(argc, argv, "o:l:")) != -1) { // Loops through command-line arguments.
		if (opt == 'o') {           // Checks if the output option was provided.
			if (strcmp(optarg, "syslog") == 0) { // Checks if the output target is syslog.
				log_to_syslog = 1; // Enables syslog output.
			} else {            // Handles the case where output is a file path.
				log_file = fopen(optarg, "a"); // Opens the specified file in append mode.
				if (!log_file) // Checks if the file failed to open.
					return EXIT_FAILURE; // Exits the program with a failure status.
			}                   // Ends the else block.
		} else if (opt == 'l') {    // Checks if the log level option was provided.
			log_level = atoi(optarg); // Converts and sets the requested log level.
		}                           // Ends the else-if block.
	}                                   // Ends the while loop.
	
	if (daemon_init() < 0)              // Attempts to daemonize the process and checks for errors.
		return EXIT_FAILURE;        // Exits with failure if daemonization fails.

	setup_signals();                    // Configures the signal handlers.

	if (log_to_syslog)                  // Checks if syslog was enabled during parsing.
		openlog("mydaemon", LOG_PID, LOG_DAEMON); // Opens a connection to the system logger.

	do_log(1, "[INFO] Daemon started.\n"); // Logs a message indicating successful startup.

	/* Ana arka plan dongusu */         
	while (daemon_running) {            // Loops continuously while the running flag is active.
		if (daemon_reload) {        // Checks if a reload signal was received.
			do_log(1, "[INFO] Reloading config - SIGHUP received.\n"); // Logs the reload event.
			daemon_reload = 0;  // Resets the reload flag.
		}                           // Ends the if block.

		sleep(5);                   // Suspends execution for 5 seconds to simulate work.
		do_log(2, "[DEBUG] Daemon is running.\n"); // Logs a debug message periodically.
	}                                   // Ends the while loop.

	do_log(1, "[INFO] Shutdown signal received. Daemon exiting.\n"); // Logs the shutdown event.

	if (log_to_syslog)                  // Checks if syslog was used.
		closelog();                 // Closes the connection to the system logger.
	if (log_file)                       // Checks if a log file was opened.
		fclose(log_file);           // Closes the open log file descriptor.

	return EXIT_SUCCESS;                // Exits the program successfully.
}                                           