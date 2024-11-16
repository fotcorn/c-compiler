#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "lexer.h"
#include "parser.h"

int main(int argc, char *argv[]) {
    char *input = NULL;
    size_t length = 0;

    if (argc < 2) {
        fprintf(stderr, "Usage: %s <path_to_file>\n", argv[0]);
        return 1;
    }

    // Open the file specified by the user
    FILE *file = fopen(argv[1], "r");
    if (!file) {
        fprintf(stderr, "Error: could not open file '%s'\n", argv[1]);
        return 1;
    }

    // Determine the file size
    fseek(file, 0, SEEK_END);
    length = ftell(file);
    rewind(file);

    // Allocate memory for the file content
    input = malloc(length + 1);
    if (!input) {
        fprintf(stderr, "Error: memory allocation failed\n");
        fclose(file);
        return 1;
    }

    // Read the file content into the input buffer
    size_t read_length = fread(input, 1, length, file);
    input[read_length] = '\0';

    fclose(file);

    struct TokenArray tokens;
    int status = lex(input, strlen(input), &tokens);
    if (status != 0) {
        free(input);
        return 1;
    }

    printf("Tokens:\n");
    print_tokens(tokens, input);
    printf("\n");

    // Call the parser
    struct ASTNode *ast = parse(&tokens);
    if (!ast) {
        fprintf(stderr, "Parsing failed\n");
        free(tokens.tokens);
        free(input);
        return 1;
    }

    // Print the AST
    printf("AST:\n");
    print_ast(ast, 0);

    // Free resources
    free_ast(ast);
    free(tokens.tokens);
    free(input);
    return 0;
}
