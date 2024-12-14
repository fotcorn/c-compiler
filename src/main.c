#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "lexer.h"
#include "parser.h"
#include "sema.h"
#include "codegen.h"

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
  struct ASTNode *ast = parse(&tokens, input);
  if (!ast) {
    fprintf(stderr, "Parsing failed\n");
    free(tokens.tokens);
    free(input);
    return 1;
  }

  // Print the AST
  printf("AST:\n");
  print_ast(ast, 0);

  // Perform semantic analysis
  printf("\nPerforming semantic analysis...\n");
  int sema_status = analyze_program(ast);
  if (sema_status != 0) {
    fprintf(stderr, "Semantic analysis failed\n");
    free_ast(ast);
    free(tokens.tokens);
    free(input);
    return 1;
  }
  printf("Semantic analysis completed successfully\n");

  // Generate assembly code
  printf("\nGenerating assembly code...\n");
  struct Assembly *assembly = generate_code(ast);
  
  // Create output file name by replacing .c with .s
  char *output_file = strdup(argv[1]);
  char *dot = strrchr(output_file, '.');
  if (dot) *dot = '\0';
  strcat(output_file, ".s");
  
  // Open output file
  FILE *out = fopen(output_file, "w");
  if (!out) {
    fprintf(stderr, "Error: could not create output file '%s'\n", output_file);
    free(output_file);
    free_ast(ast);
    free(tokens.tokens);
    free(input);
    return 1;
  }

  // Write assembly to file
  print_assembly(out, assembly);
  fclose(out);
  printf("Assembly code written to %s\n", output_file);
  free(output_file);

  // Free resources
  free_ast(ast);
  free(tokens.tokens);
  free(input);
  return 0;
}
