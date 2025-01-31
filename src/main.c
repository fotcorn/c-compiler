#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "lexer.h"
#include "parser.h"
#include "sema.h"
#include "codegen.h"
#include "print_assembly.h"
#include "print_tokens.h"
#include "print_ast.h"
#include "print_sema.h"

int main(int argc, char *argv[]) {
  char *input = NULL;
  size_t length = 0;

  bool print_tokens_flag = false;
  bool print_ast_flag = false;
  bool print_sema_flag = false;
  char *filename = NULL;
  int flag_count = 0;

  // Parse command line arguments
  for (int i = 1; i < argc; i++) {
      if (strcmp(argv[i], "--print-tokens") == 0) {
          print_tokens_flag = true;
          flag_count++;
      } else if (strcmp(argv[i], "--print-ast") == 0) {
          print_ast_flag = true;
          flag_count++;
      } else if (strcmp(argv[i], "--print-sema") == 0) {
          print_sema_flag = true;
          flag_count++;
      } else {
          if (filename != NULL) {
              fprintf(stderr, "Error: Multiple input files specified\n");
              return 1;
          }
          filename = argv[i];
      }
  }

  if (filename == NULL) {
      fprintf(stderr, "Usage: %s [--print-tokens] [--print-ast] [--print-sema] <file>\n", argv[0]);
      return 1;
  }

  if (flag_count > 1) {
      fprintf(stderr, "Error: Only one print flag can be specified\n");
      return 1;
  }

  // Open the file specified by the user
  FILE *file = fopen(filename, "r");
  if (!file) {
      fprintf(stderr, "Error: could not open file '%s'\n", filename);
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

  if (print_tokens_flag) {
      print_tokens(tokens, input);
      free(input);
      free(tokens.tokens);
      return 0;
  }

  // Call the parser
  struct ASTNode *ast = parse(&tokens, input);
  if (!ast) {
      fprintf(stderr, "Parsing failed\n");
      free(tokens.tokens);
      free(input);
      return 1;
  }

  if (print_ast_flag) {
      print_ast(ast, 0);
      free_ast(ast);
      free(tokens.tokens);
      free(input);
      return 0;
  }

  // Perform semantic analysis
  struct SemanticContext *sema_context = analyze_program(ast);
  if (!sema_context) {
      fprintf(stderr, "Semantic analysis failed\n");
      free_ast(ast);
      free(tokens.tokens);
      free(input);
      return 1;
  }

  if (print_sema_flag) {
      print_semantic_context(sema_context);
      free_ast(ast);
      free(tokens.tokens);
      free(input);
      // Note: Should properly free sema_context here - requires additional cleanup functions
      return 0;
  }

  // Generate assembly code
  struct Assembly *assembly = generate_code(ast, sema_context);
  
  // Write assembly to stdout
  print_assembly(stdout, assembly);

  // Free resources
  free_ast(ast);
  free(tokens.tokens);
  free(input);
  return 0;
}
