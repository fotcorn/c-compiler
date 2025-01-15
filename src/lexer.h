#include "common.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Structure to store defined constants
struct Define {
  char *name;
  int start;
  int end;
};

struct DefineArray {
  struct Define *defines;
  int capacity;
  int count;
};

struct DefineArray create_define_array(void) {
  struct DefineArray arr;
  arr.capacity = 8;
  arr.count = 0;
  arr.defines = malloc(arr.capacity * sizeof(struct Define));
  return arr;
}

void add_define(struct DefineArray *arr, const char *name, int start, int end) {
  if (arr->count >= arr->capacity) {
    arr->capacity *= 2;
    arr->defines = realloc(arr->defines, arr->capacity * sizeof(struct Define));
  }
  arr->defines[arr->count].name = strdup(name);
  arr->defines[arr->count].start = start;
  arr->defines[arr->count].end = end;
  arr->count++;
}

int get_define_start_end(struct DefineArray *arr, const char *name, int len, int *start, int *end) {
  for (int i = 0; i < arr->count; i++) {
    if (strncmp(arr->defines[i].name, name, len) == 0) {
      *start = arr->defines[i].start;
      *end = arr->defines[i].end;
      return 1;
    }
  }
  return 0;
}

void free_define_array(struct DefineArray *arr) {
  for (int i = 0; i < arr->count; i++) {
    free(arr->defines[i].name);
  }
  free(arr->defines);
}

struct TokenArray create_token_array(void) {
  struct TokenArray arr;
  arr.capacity = 8;
  arr.count = 0;
  arr.tokens = malloc(arr.capacity * sizeof(struct Token));
  return arr;
}

void add_token(struct TokenArray *arr, struct Token token) {
  if (arr->count >= arr->capacity) {
    arr->capacity *= 2;
    arr->tokens = realloc(arr->tokens, arr->capacity * sizeof(struct Token));
  }
  arr->tokens[arr->count++] = token;
}

int lex(char *input, int length, struct TokenArray *tokens) {
  *tokens = create_token_array();
  struct DefineArray defines = create_define_array();
  int i = 0;
  int line = 1;

  while (i < length) {
    // Skip whitespace
    while (i < length &&
           (input[i] == ' ' || input[i] == '\n' || input[i] == '\t')) {
      if (input[i] == '\n')
        line++;
      i++;
    }
    if (i >= length)
      break;

    // Handle #define directives
    if (input[i] == '#') {
      i++; // Skip #
      // Skip whitespace after #
      while (i < length && (input[i] == ' ' || input[i] == '\t')) {
        i++;
      }

      // Check if it's a define directive
      if (i + 5 < length && strncmp(&input[i], "define", 6) == 0) {
        i += 6; // Skip "define"

        // Skip whitespace after define
        while (i < length && (input[i] == ' ' || input[i] == '\t')) {
          i++;
        }

        // Get constant name
        int name_start = i;
        while (i < length && (isalnum(input[i]) || input[i] == '_')) {
          i++;
        }
        int name_len = i - name_start;
        char *name = strndup(&input[name_start], name_len);

        // Skip whitespace between name and value
        while (i < length && (input[i] == ' ' || input[i] == '\t')) {
          i++;
        }

        // Get position of constant value
        int value_start = i;
        while (i < length && isdigit(input[i])) {
          i++;
        }

        // Add to defines array
        add_define(&defines, name, value_start, i);
        free(name);

        // Skip to end of line
        while (i < length && input[i] != '\n') {
          i++;
        }
        if (i < length && input[i] == '\n') {
          line++;
          i++;
        }
        continue;
      }
    }

    // Handle comments
    if (i + 1 < length && input[i] == '/') {
      if (input[i + 1] == '/') {
        // Single-line comment
        while (i < length && input[i] != '\n') {
          i++;
        }
        if (i < length && input[i] == '\n')
          line++;
        continue;
      } else if (input[i + 1] == '*') {
        // Multi-line comment
        i += 2;  // Skip /*
        while (i + 1 < length && !(input[i] == '*' && input[i + 1] == '/')) {
          if (input[i] == '\n')
            line++;
          i++;
        }
        if (i + 1 >= length) {
          fprintf(stderr, "Line %d: Error: Unterminated multi-line comment\n", line);
          return 1;
        }
        i += 2;  // Skip */
        continue;
      }
    }

    // Create token with current position
    struct Token token;
    token.start = i;
    token.line = line;

    // Single character tokens
    if (input[i] == '{') {
      token.type = TOKEN_LEFT_BRACE;
      i++;
    } else if (input[i] == '}') {
      token.type = TOKEN_RIGHT_BRACE;
      i++;
    } else if (input[i] == '(') {
      token.type = TOKEN_LEFT_PAREN;
      i++;
    } else if (input[i] == ')') {
      token.type = TOKEN_RIGHT_PAREN;
      i++;
    } else if (input[i] == ';') {
      token.type = TOKEN_SEMICOLON;
      i++;
    } else if (input[i] == ',') {
      token.type = TOKEN_COMMA;
      i++;
    } else if (input[i] == '.') {
      token.type = TOKEN_PERIOD;
      i++;
    } else if (input[i] == '[') {
      token.type = TOKEN_LEFT_BRACKET;
      i++;
    } else if (input[i] == ']') {
      token.type = TOKEN_RIGHT_BRACKET;
      i++;
    }
    // Arithmetic operators
    else if (input[i] == '+') {
      token.type = TOKEN_PLUS;
      i++;
    } else if (input[i] == '-') {
      token.type = TOKEN_MINUS;
      i++;
    } else if (input[i] == '*') {
      token.type = TOKEN_MULTIPLY;
      i++;
    } else if (input[i] == '/') {
      token.type = TOKEN_DIVIDE;
      i++;
    }
    // Comparison operators
    else if (input[i] == '=') {
      i++;
      if (i < length && input[i] == '=') {
        token.type = TOKEN_EQUAL_EQUAL;
        i++;
      } else {
        token.type = TOKEN_EQUAL;
      }
    } else if (input[i] == '!') {
      i++;
      if (i < length && input[i] == '=') {
        token.type = TOKEN_NOT_EQUAL;
        i++;
      } else {
        fprintf(stderr, "Line %d: Error: Expected '=' after '!' at position %d\n", line,
               i);
        return 1;
      }
    } else if (input[i] == '<') {
      i++;
      if (i < length && input[i] == '=') {
        token.type = TOKEN_LESS_EQUAL;
        i++;
      } else {
        token.type = TOKEN_LESS;
      }
    } else if (input[i] == '>') {
      i++;
      if (i < length && input[i] == '=') {
        token.type = TOKEN_GREATER_EQUAL;
        i++;
      } else {
        token.type = TOKEN_GREATER;
      }
    } else if (input[i] == '|') {
      i++;
      if (i < length && input[i] == '|') {
        token.type = TOKEN_LOGICAL_OR;
        i++;
      } else {
        fprintf(stderr, "Line %d: Error: Expected '|' after '|' at position %d\n", line,
               i);
        return 1;
      }
    } else if (input[i] == '&') {
      i++;
      if (i < length && input[i] == '&') {
        token.type = TOKEN_LOGICAL_AND;
        i++;
      } else {
        token.type = TOKEN_AMPERSAND;
      }
    }
    // String literals
    else if (input[i] == '"') {
      token.type = TOKEN_LITERAL_STRING;
      i++;
      while (i < length && input[i] != '"') {
        if (input[i] == '\n')
          line++;
        if (input[i] == '\\' && i + 1 < length) {
          i += 2;
        } else {
          i++;
        }
      }
      if (i < length && input[i] == '"') {
        i++;
      } else {
        fprintf(stderr, "Line %d: Error: Unterminated string at position %d\n", line,
               token.start);
        return 1;
      }
    }
    // Character literals
    else if (input[i] == '\'') {
      token.type = TOKEN_LITERAL_CHAR;
      i++;
      if (i < length && input[i] == '\\') {
        i += 2;
      } else {
        i++;
      }
      if (i < length && input[i] == '\'') {
        i++;
      } else {
        fprintf(
            stderr,
            "Line %d: Error: Unterminated character literal at position %d\n",
            line, token.start);
        return 1;
      }
    }
    // Numbers
    else if (isdigit(input[i])) {
      token.type = TOKEN_LITERAL_INT;
      while (i < length && isdigit(input[i])) {
        i++;
      }
    }
    // Identifiers and keywords
    else if (isalpha(input[i]) || input[i] == '_') {
      while (i < length && (isalnum(input[i]) || input[i] == '_')) {
        i++;
      }

      int len = i - token.start;
      if (strncmp(&input[token.start], "return", len) == 0 && len == 6) {
        token.type = TOKEN_RETURN;
      } else if (strncmp(&input[token.start], "if", len) == 0 && len == 2) {
        token.type = TOKEN_IF;
      } else if (strncmp(&input[token.start], "else", len) == 0 && len == 4) {
        token.type = TOKEN_ELSE;
      } else if (strncmp(&input[token.start], "while", len) == 0 && len == 5) {
        token.type = TOKEN_WHILE;
      } else if (strncmp(&input[token.start], "struct", len) == 0 && len == 6) {
        token.type = TOKEN_STRUCT;
      } else if (get_define_start_end(&defines, &input[token.start], len, &token.start, &token.end)) {
        token.type = TOKEN_LITERAL_INT;
      } else {
        token.type = TOKEN_IDENTIFIER;
      }
    }
    // Unexpected characters
    else {
      fprintf(stderr, "Line %d: Error: Unexpected character '%c' at position %d\n", line,
             input[i], i);
      return 1;
    }

    token.end = i;
    add_token(tokens, token);
  }

  free_define_array(&defines);
  return 0;
}
