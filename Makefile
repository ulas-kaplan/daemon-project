# compiler and flags
CC = gcc
CFLAGS = -Wall -Wextra -O2

# output target
TARGET = mydaemon
LOGFILE = /tmp/daemon.log

# build rules
all: $(TARGET)

# we now have 3 object files (.o) that need to be linked together
$(TARGET): main.o daemon.o log.o
	$(CC) $(CFLAGS) -o $(TARGET) main.o daemon.o log.o

# the % symbol is a pattern rule that automatically compiles each .c to a .o
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# clean rule (removes all .o files, the executable, and the log file)
clean:
	rm -f *.o $(TARGET) $(LOGFILE)

# TEST COMMANDS
run: all
	./$(TARGET) -o $(LOGFILE) -l 2
	@echo "Daemon started. To watch the log, run: make watch"

watch:
	tail -f $(LOGFILE)

stop:
	@pkill -x -TERM $(TARGET) && echo "Daemon stopped." || echo "No running daemon found."

reload:
	@pkill -x -HUP $(TARGET) && echo "Config reloaded." || echo "No running daemon found."

.PHONY: all clean run watch stop reload