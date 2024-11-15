CC = gcc
CFLAGS = -Wall -Wextra -pedantic

TARGET = main

$(TARGET): src/main.c
	$(CC) $(CFLAGS) $< -o $@

.PHONY: clean

clean:
	rm -f $(TARGET)
