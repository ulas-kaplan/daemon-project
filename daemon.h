#ifndef DAEMON_H                            // Header guard to prevent multiple inclusions.
#define DAEMON_H                            // Defines the macro for the header guard.

/* Sinyal isleyiciler tarafindan degistirilen durumlar */ // Keeps original Turkish comment.
extern volatile int daemon_running;         // Declares the global running flag modified by signals.
extern volatile int daemon_reload;          // Declares the global reload flag modified by signals.

int daemon_init(void);                      // Declares the function to initialize the daemon.
void setup_signals(void);                   // Declares the function to configure signal handlers.

#endif /* DAEMON_H */                       // Closes the header guard.