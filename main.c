#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

int TOKEN_LEFT_BRACE = 1;
int TOKEN_RIGHT_BRACE = 2;
int TOKEN_LEFT_PAREN = 3;
int TOKEN_RIGHT_PAREN = 4;
int TOKEN_LEFT_BRACKET = 5;
int TOKEN_RIGHT_BRACKET = 6;
int TOKEN_LITERAL_INT = 7;
int TOKEN_LITERAL_CHAR = 8;
int TOKEN_LITERAL_STRING = 9;
int TOKEN_IDENTIFIER = 10;
int TOKEN_RETURN = 11;
int TOKEN_IF = 12;
int TOKEN_ELSE = 13;
int TOKEN_WHILE = 14;
int TOKEN_STRUCT = 15;
int TOKEN_SEMICOLON = 16;
int TOKEN_COMMA = 17;
int TOKEN_PLUS = 18;
int TOKEN_MINUS = 19;
int TOKEN_MULTIPLY = 20;
int TOKEN_DIVIDE = 21;
int TOKEN_EQUAL = 22;
int TOKEN_EQUAL_EQUAL = 23;
int TOKEN_NOT_EQUAL = 24;
int TOKEN_LESS = 25;
int TOKEN_LESS_EQUAL = 26;
int TOKEN_GREATER = 27;
int TOKEN_GREATER_EQUAL = 28;
int TOKEN_PERIOD = 29;
int TOKEN_LOGICAL_OR = 30;
int TOKEN_LOGICAL_AND = 31;
int TOKEN_AMPERSAND = 32;

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

int lex(char *input, int length, struct TokenArray *tokens) {
    *tokens = create_token_array();
    int i = 0;
    int line = 1;

    while (i < length) {
        // Skip whitespace
        while (i < length && (input[i] == ' ' || input[i] == '\n' || input[i] == '\t')) {
            if (input[i] == '\n') line++;
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
            if (i < length && input[i] == '\n') line++;
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
                printf("Line %d: Error: Expected '=' after '!' at position %d\n", line, i);
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
        }
        else if (input[i] == '|') {
            i++;
            if (i < length && input[i] == '|') {
                token.type = TOKEN_LOGICAL_OR;
                i++;
            } else {
                printf("Line %d: Error: Expected '|' after '|' at position %d\n", line, i);
                return 1;
            }
        }
        else if (input[i] == '&') {
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
                if (input[i] == '\n') line++;
                if (input[i] == '\\' && i + 1 < length) {
                    i += 2;
                } else {
                    i++;
                }
            }
            if (i < length && input[i] == '"') {
                i++;
            } else {
                printf("Line %d: Error: Unterminated string at position %d\n", line, token.start);
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
                printf("Line %d: Error: Unterminated character literal at position %d\n", line, token.start);
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
            printf("Line %d: Error: Unexpected character '%c' at position %d\n", line, input[i], i);
            return 1;
        }

        token.end = i;
        add_token(tokens, token);
    }

    return 0;
}

int main() {
    char *input = NULL;
    size_t capacity = 0;
    size_t length = 0;
    char buffer[1024];
    
    while (fgets(buffer, sizeof(buffer), stdin) != NULL) {
        size_t chunk_len = strlen(buffer);
        if (length + chunk_len + 1 > capacity) {
            if (capacity == 0) {
                capacity = 1024;
            } else {
                capacity = capacity * 2;
            }
            input = realloc(input, capacity);
            if (input == NULL) {
                printf("Error: memory allocation failed\n");
                return 1;
            }
        }
        strcpy(input + length, buffer);
        length += chunk_len;
    }
    
    if (input == NULL) {
        printf("Error: no input received\n");
        return 1;
    }
    struct TokenArray tokens;
    int status = lex(input, strlen(input), &tokens);
    if (status != 0) {
        free(input);
        return 1;
    }

    int i = 0;
    while (i < tokens.count) {
        struct Token token = tokens.tokens[i];
        printf("Token %d: ", i);
        
        if (token.type == TOKEN_IDENTIFIER || token.type == TOKEN_LITERAL_INT || token.type == TOKEN_LITERAL_STRING) {
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
    return 0;
}
