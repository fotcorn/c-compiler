#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

int TOKEN_LEFT_BRACE = 1;
int TOKEN_RIGHT_BRACE = 2;
int TOKEN_LEFT_PAREN = 3;
int TOKEN_RIGHT_PAREN = 4;
int TOKEN_LITERAL_INT = 5;
int TOKEN_LITERAL_CHAR = 6;
int TOKEN_LITERAL_STRING = 7;
int TOKEN_IDENTIFIER = 8;
int TOKEN_RETURN = 9;
int TOKEN_IF = 10;
int TOKEN_ELSE = 11;
int TOKEN_WHILE = 12;
int TOKEN_STRUCT = 13;
int TOKEN_SEMICOLON = 14;
int TOKEN_COMMA = 15;
int TOKEN_PLUS = 16;
int TOKEN_MINUS = 17;
int TOKEN_MULTIPLY = 18;
int TOKEN_DIVIDE = 19;
int TOKEN_EQUAL = 20;
int TOKEN_EQUAL_EQUAL = 21;
int TOKEN_NOT_EQUAL = 22;
int TOKEN_LESS = 23;
int TOKEN_LESS_EQUAL = 24;
int TOKEN_GREATER = 25;
int TOKEN_GREATER_EQUAL = 26;

struct Token {
    int type;
    int start;
    int end;
};

struct TokenArray {
    struct Token *tokens;
    int capacity;
    int count;
};

struct TokenArray create_token_array() {
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

struct TokenArray lex(char *input, int length) {
    struct TokenArray tokens = create_token_array();
    int i = 0;

    while (i < length) {
        // Skip whitespace
        while (i < length && (input[i] == ' ' || input[i] == '\n' || input[i] == '\t')) {
            i++;
        }
        if (i >= length) break;

        // Create token with current position
        struct Token token;
        token.start = i;

        // Handle comments
        if (i + 1 < length && input[i] == '/' && input[i + 1] == '/') {
            while (i < length && input[i] != '\n') {
                i++;
            }
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
                printf("Error: Expected '=' after '!' at position %d\n", i);
                continue;
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
        }
        // String literals
        else if (input[i] == '"') {
            token.type = TOKEN_LITERAL_STRING;
            i++;
            while (i < length && input[i] != '"') {
                if (input[i] == '\\' && i + 1 < length) {
                    i += 2;
                } else {
                    i++;
                }
            }
            if (i < length && input[i] == '"') {
                i++;
            } else {
                printf("Error: Unterminated string at position %d\n", token.start);
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
                printf("Error: Unterminated character literal at position %d\n", token.start);
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
            printf("Error: Unexpected character '%c' at position %d\n", input[i], i);
            i++;
            continue;
        }

        token.end = i;
        add_token(&tokens, token);
    }

    return tokens;
}

int main() {
    char *input = "int main() { return 0; }";
    struct TokenArray tokens = lex(input, strlen(input));
    return 0;
}
