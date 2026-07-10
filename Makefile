# compiler and flags
CC = gcc
CFLAGS = -Wall -Wextra -O2

# output target
TARGET = mydaemon

# build rules
all: $(TARGET)

$(TARGET): main.o
	$(CC) $(CFLAGS) -o $(TARGET) main.o

main.o: main.c
	$(CC) $(CFLAGS) -c main.c

# clean rule
clean:
	rm -f *.o $(TARGET)

.PHONY: all clean