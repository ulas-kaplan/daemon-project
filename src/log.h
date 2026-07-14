#ifndef LOG_H                               // Header guard to prevent double inclusion.
#define LOG_H                               // Defines the macro for the header guard.

#include <stdio.h>                          // Includes standard I/O library for FILE type.

extern int log_level;                       // Declares the global log level variable.
extern int log_to_syslog;                   // Declares the global flag for syslog usage.
extern FILE *log_file;                      // Declares the global file pointer for logging.

void do_log(int level, const char *msg);    // Declares the function for writing log messages.

#endif /* LOG_H */                          // Closes the header guard.