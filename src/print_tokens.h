#include "common.h"
#include <stdio.h>
#include <string.h>

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
