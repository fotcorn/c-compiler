CC = gcc
CFLAGS = -Wall -Wextra -pedantic -g

TARGET = main
HEADERS = src/lexer.h src/parser.h src/common.h

$(TARGET): src/main.c $(HEADERS)
	$(CC) $(CFLAGS) $< -o $@

.PHONY: clean

clean:
	rm -f $(TARGET)
