#include "common.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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

    // Create token with current position
    struct Token token;
    token.start = i;
    token.line = line;

    // Handle comments
    if (i + 1 < length && input[i] == '/' && input[i + 1] == '/') {
      while (i < length && input[i] != '\n') {
        i++;
      }
      if (i < length && input[i] == '\n')
        line++;
      continue;
    }

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
        printf("Line %d: Error: Expected '=' after '!' at position %d\n", line,
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
        printf("Line %d: Error: Expected '|' after '|' at position %d\n", line,
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
        printf("Line %d: Error: Unterminated string at position %d\n", line,
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
        printf(
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
      } else {
        token.type = TOKEN_IDENTIFIER;
      }
    }
    // Unexpected characters
    else {
      printf("Line %d: Error: Unexpected character '%c' at position %d\n", line,
             input[i], i);
      return 1;
    }

    token.end = i;
    add_token(tokens, token);
  }

  return 0;
}

void print_tokens(struct TokenArray tokens, const char *input) {
  int i = 0;
  while (i < tokens.count) {
    struct Token token = tokens.tokens[i];
    printf("Token %d: ", i);

    if (token.type == TOKEN_IDENTIFIER || token.type == TOKEN_LITERAL_INT ||
        token.type == TOKEN_LITERAL_STRING) {
      int len = token.end - token.start;
      char content[256];
      strncpy(content, &input[token.start], len);
      content[len] = '\0';

      if (token.type == TOKEN_IDENTIFIER) {
        printf("IDENTIFIER '%s'\n", content);
      } else if (token.type == TOKEN_LITERAL_INT) {
        printf("INTEGER %s\n", content);
      } else {
        printf("STRING '%s'\n", content);
      }
    } else if (token.type == TOKEN_LEFT_BRACE) {
      printf("{\n");
    } else if (token.type == TOKEN_RIGHT_BRACE) {
      printf("}\n");
    } else if (token.type == TOKEN_LEFT_PAREN) {
      printf("(\n");
    } else if (token.type == TOKEN_RIGHT_PAREN) {
      printf(")\n");
    } else if (token.type == TOKEN_SEMICOLON) {
      printf(";\n");
    } else if (token.type == TOKEN_COMMA) {
      printf(",\n");
    } else if (token.type == TOKEN_PLUS) {
      printf("+\n");
    } else if (token.type == TOKEN_MINUS) {
      printf("-\n");
    } else if (token.type == TOKEN_MULTIPLY) {
      printf("*\n");
    } else if (token.type == TOKEN_DIVIDE) {
      printf("/\n");
    } else if (token.type == TOKEN_EQUAL) {
      printf("=\n");
    } else if (token.type == TOKEN_EQUAL_EQUAL) {
      printf("==\n");
    } else if (token.type == TOKEN_NOT_EQUAL) {
      printf("!=\n");
    } else if (token.type == TOKEN_LESS) {
      printf("<\n");
    } else if (token.type == TOKEN_LESS_EQUAL) {
      printf("<=\n");
    } else if (token.type == TOKEN_GREATER) {
      printf(">\n");
    } else if (token.type == TOKEN_GREATER_EQUAL) {
      printf(">=\n");
    } else if (token.type == TOKEN_RETURN) {
      printf("return\n");
    } else if (token.type == TOKEN_IF) {
      printf("if\n");
    } else if (token.type == TOKEN_ELSE) {
      printf("else\n");
    } else if (token.type == TOKEN_WHILE) {
      printf("while\n");
    } else if (token.type == TOKEN_STRUCT) {
      printf("struct\n");
    } else if (token.type == TOKEN_PERIOD) {
      printf(".\n");
    } else if (token.type == TOKEN_LOGICAL_OR) {
      printf("||\n");
    } else if (token.type == TOKEN_LOGICAL_AND) {
      printf("&&\n");
    } else if (token.type == TOKEN_LEFT_BRACKET) {
      printf("[\n");
    } else if (token.type == TOKEN_RIGHT_BRACKET) {
      printf("]\n");
    } else if (token.type == TOKEN_AMPERSAND) {
      printf("&\n");
    } else {
      printf("OTHER TOKEN TYPE %d\n", token.type);
    }
    i++;
  }
}
