# compiler and flags
CC = gcc
CFLAGS = -Wall -Wextra -O2

# directories
SRC_DIR = src

# output target
TARGET = mydaemon
LOGFILE = /tmp/daemon.log

# source and object files
SRCS = $(SRC_DIR)/main.c $(SRC_DIR)/daemon.c $(SRC_DIR)/log.c
OBJS = $(SRCS:.c=.o)

# build rules
all: $(TARGET)

# linking
$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS)

# compiling (pattern rule for files inside src/)
$(SRC_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

# clean rule
clean:
	rm -f $(SRC_DIR)/*.o $(TARGET) $(LOGFILE)

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