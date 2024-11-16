CC = gcc
CFLAGS = -Wall -Wextra -pedantic -g

TARGET = main

$(TARGET): src/main.c
	$(CC) $(CFLAGS) $< -o $@

.PHONY: clean

clean:
	rm -f $(TARGET)
