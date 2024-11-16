#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "lexer.h"
#include "parser.h"

int main(void) {
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

    printf("Tokens:\n");
    print_tokens(tokens, input);
    printf("\n");

    status = parse(&tokens, input);
    if (status != 0) {
        fprintf(stderr, "Parsing failed\n");
        free(input);
        free(tokens.tokens);
        return 1;
    }

    free(tokens.tokens);
    free(input);
    return 0;
}
