TARGET = input_recorder
LIBS = -L/usr/lib/x86_64-linux-gnu -lX11
CC = gcc
CFLAGS = -g -Wall -std=c99 -D_XOPEN_SOURCE=600 -D_BSD_SOURCE

SOURCES = $(wildcard ./src/*.c)
OBJECTS = $(patsubst %.c, %.o, $(SOURCES))
HEADERS = $(wildcard ./src/*.h)

%.o: %.c $(HEADERS)
	$(CC) $(CFLAGS) -c $< -o $@

$(TARGET): $(OBJECTS)
	$(CC) $(OBJECTS) -Wall $(LIBS) -o $@

.PHONY: clean
clean:
	rm -f ./src/*.o
	rm -f $(TARGET)
