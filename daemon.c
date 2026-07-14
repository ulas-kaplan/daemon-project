#define _XOPEN_SOURCE 700                   // Exposes POSIX standard structures like sigaction.

#include "daemon.h"                         // Includes the header for daemon declarations.
#include "log.h"                            // Includes the logging utilities.

#include <stdlib.h>                         // Includes standard library functions like exit.
#include <unistd.h>                         // Includes POSIX API functions like fork and setsid.
#include <signal.h>                         // Includes signal handling definitions.
#include <sys/stat.h>                       // Includes file status functions like umask.
#include <fcntl.h>                          // Includes file control options like open.

volatile int daemon_running = 1;            // Initializes the global flag for the main loop.
volatile int daemon_reload = 0;             // Initializes the global flag for configuration reload.

static void handle_sig(int sig)             // Implements the signal handler function.
{                                           // Begins the function body.
	if (sig == SIGTERM || sig == SIGINT)// Checks for termination or interrupt signals.
		daemon_running = 0;         // Stops the daemon by resetting the run flag.
	else if (sig == SIGHUP)             // Checks for the hang-up signal.
		daemon_reload = 1;          // Triggers a configuration reload.
}                                           // Ends the function body.

void setup_signals(void)                    // Implements the signal configuration setup.
{                                           // Begins the function body.
	struct sigaction sa;                // Declares a sigaction structure for signal setup.

	sa.sa_handler = handle_sig;         // Assigns the custom handler function.
	sigemptyset(&sa.sa_mask);           // Clears the signal mask to block no additional signals.
	sa.sa_flags = 0;                    // Sets default signal behavior flags.

	sigaction(SIGTERM, &sa, NULL);      // Registers the handler for the SIGTERM signal.
	sigaction(SIGINT, &sa, NULL);       // Registers the handler for the SIGINT signal.
	sigaction(SIGHUP, &sa, NULL);       // Registers the handler for the SIGHUP signal.
}                                           // Ends the function body.

int daemon_init(void)                       // Implements the daemonization process.
{                                           // Begins the function body.
	pid_t pid;                          // Declares a variable to store the process ID.
	int i;                              // Declares a loop counter variable.
	int fd;                             // Declares a file descriptor variable.

	pid = fork();                       // Forks the current process to create a child.
	if (pid < 0)                        // Checks if the first fork failed.
		return -1;                  // Returns an error code on failure.
	if (pid > 0)                        // Checks if this is the parent process.
		exit(EXIT_SUCCESS);         // Exits the parent process immediately.

	if (setsid() < 0)                   // Creates a new session and detaches from the terminal.
		return -1;                  // Returns an error if session creation fails.

	pid = fork();                       // Forks again to prevent acquiring a new terminal.
	if (pid < 0)                        // Checks if the second fork failed.
		return -1;                  // Returns an error code on failure.
	if (pid > 0)                        // Checks if this is the first child process.
		exit(EXIT_SUCCESS);         // Exits the first child, leaving the grandchild orphaned.

	umask(0027);                        // Sets the file mode creation mask to restrict permissions.
	
	if (chdir("/") < 0)                 // Changes the working directory to the root directory.
		return -1;                  // Returns an error if the directory change fails.

	/* Dosya tanimlayicilarini (file descriptors) kapat */ // Keeps original Turkish comment.
	for (i = 0; i < 1024; i++) {        // Loops through all possible file descriptors.
		if (log_file && i == fileno(log_file)) // Checks if the descriptor belongs to our log file.
			continue;           // Skips closing the active log file descriptor.
		close(i);                   // Closes the current file descriptor.
	}                                   // Ends the for loop.

	fd = open("/dev/null", O_RDWR);     // Opens /dev/null for reading and writing.
	if (fd >= 0) {                      // Checks if /dev/null was successfully opened.
		dup2(fd, STDIN_FILENO);     // Redirects standard input to /dev/null.
		dup2(fd, STDOUT_FILENO);    // Redirects standard output to /dev/null.
		dup2(fd, STDERR_FILENO);    // Redirects standard error to /dev/null.
		if (fd > 2)                 // Checks if the original descriptor is not standard I/O.
			close(fd);          // Closes the extra file descriptor.
	}                                   // Ends the if block.

	return 0;                           // Returns zero indicating successful daemonization.
}                                           // Ends the function body.