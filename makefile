CC=gcc
CFLAGS=-Wall
TARGET=UnixLs

all: $(TARGET)

$(TARGET): UnixLs.c
	$(CC) $(CFLAGS) -o $(TARGET) UnixLs.c

clean:
	rm -f $(TARGET)

.PHONY: all clean
