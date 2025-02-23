#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Parser state
struct Parser {
  struct TokenArray *tokens;
  int position;
  const char *input; // Source code input string
};

// Forward declarations
static struct ASTNode *parse_program(struct Parser *parser);
static struct ASTNode *parse_function_declaration(struct Parser *parser);
static struct ASTNode *parse_statement(struct Parser *parser);
static struct ASTNode *parse_expression(struct Parser *parser);
static struct ASTNode *parse_term(struct Parser *parser);
static struct ASTNode *parse_factor(struct Parser *parser);
static struct ASTNode *parse_primary(struct Parser *parser);
static int match(struct Parser *parser, int token_type);
static struct Token *peek(struct Parser *parser);
static struct Token *previous(struct Parser *parser);
static struct Token *advance(struct Parser *parser);
static int is_at_end(struct Parser *parser);
static void expect(struct Parser *parser, int token_type, const char *message);
static struct ASTNode *parse_arguments(struct Parser *parser);
static struct ASTNode *parse_equality(struct Parser *parser);
static struct ASTNode *parse_additive(struct Parser *parser);
static struct ASTNode *parse_block(struct Parser *parser);

// Implement the parse function
struct ASTNode *parse(struct TokenArray *tokens, char *input) {
  struct Parser parser;
  parser.tokens = tokens;
  parser.position = 0;
  parser.input = input;
  return parse_program(&parser);
}

// Parse a program (list of functions)
static struct ASTNode *parse_program(struct Parser *parser) {
  struct ASTNode *node = NULL;
  struct ASTNode **current = &node;

  while (!is_at_end(parser)) {
    struct ASTNode *func_decl = parse_function_declaration(parser);
    if (func_decl) {
      *current = func_decl;
      current = &((*current)->next);
    } else if (!is_at_end(parser)) {
      struct Token *current_token = peek(parser);
      fprintf(stderr, "Error on line %d: Expected function declaration.\n",
             current_token ? current_token->line : 0);
      exit(1);
    }
  }

  return node;
}

// Parse a function declaration
static struct ASTNode *parse_function_declaration(struct Parser *parser) {
  // Match return type
  if (!match(parser, TOKEN_IDENTIFIER)) {
    return NULL;
  }
  struct Token *type_token = advance(parser);
  char *return_type = strndup(&parser->input[type_token->start],
                           type_token->end - type_token->start);

  // Match function name (e.g., 'main')
  expect(parser, TOKEN_IDENTIFIER, "Expected function name.");
  struct Token *name_token = previous(parser);
  char *func_name = strndup(&parser->input[name_token->start],
                            name_token->end - name_token->start);

  // Match '('
  expect(parser, TOKEN_LEFT_PAREN, "Expected '(' after function name.");

  // Parse parameters
  struct FunctionParameter *parameters = NULL;
  int param_count = 0;
  int param_capacity = 0;

  if (!match(parser, TOKEN_RIGHT_PAREN)) {
    do {
      // Parameter type
      expect(parser, TOKEN_IDENTIFIER, "Expected parameter type.");
      struct Token *param_type_token = previous(parser);

      // Parameter name
      expect(parser, TOKEN_IDENTIFIER, "Expected parameter name.");
      struct Token *param_name_token = previous(parser);

      // Add parameter to array
      if (param_count >= param_capacity) {
        param_capacity = param_capacity == 0 ? 4 : param_capacity * 2;
        parameters = realloc(parameters, param_capacity * sizeof(struct FunctionParameter));
      }

      parameters[param_count].type = strndup(&parser->input[param_type_token->start],
                                           param_type_token->end - param_type_token->start);
      parameters[param_count].name = strndup(&parser->input[param_name_token->start],
                                           param_name_token->end - param_name_token->start);
      param_count++;
    } while (match(parser, TOKEN_COMMA) && advance(parser));
  }

  // Match ')'
  expect(parser, TOKEN_RIGHT_PAREN, "Expected ')' after parameters.");

  // Match '{'
  expect(parser, TOKEN_LEFT_BRACE, "Expected '{' before function body.");

  // Parse function body
  struct ASTNode *body = parse_block(parser);

  // Match '}'
  expect(parser, TOKEN_RIGHT_BRACE, "Expected '}' after function body.");

  // Create function declaration node
  struct ASTNode *node = malloc(sizeof(struct ASTNode));
  node->type = NODE_FUNCTION_DECLARATION;
  node->function_decl.name = func_name;
  node->function_decl.return_type = return_type;
  node->function_decl.parameters = parameters;
  node->function_decl.param_count = param_count;
  node->function_decl.body = body;
  node->next = NULL;

  return node;
}

// Parse a statement
static struct ASTNode *parse_statement(struct Parser *parser) {
  if (match(parser, TOKEN_WHILE)) {
      advance(parser); // Consume 'while'
      expect(parser, TOKEN_LEFT_PAREN, "Expected '(' after 'while'.");
      struct ASTNode *condition = parse_expression(parser);
      expect(parser, TOKEN_RIGHT_PAREN, "Expected ')' after while condition.");
      expect(parser, TOKEN_LEFT_BRACE, "Expected '{' before while body.");
      struct ASTNode *body = parse_block(parser);
      expect(parser, TOKEN_RIGHT_BRACE, "Expected '}' after while body.");
      struct ASTNode *node = malloc(sizeof(struct ASTNode));
      node->type = NODE_WHILE_STATEMENT;
      node->while_stmt.condition = condition;
      node->while_stmt.body = body;
      node->next = NULL;
      return node;
  }

  // If statement
  if (match(parser, TOKEN_IF)) {
    advance(parser); // Consume 'if'
    expect(parser, TOKEN_LEFT_PAREN, "Expected '(' after 'if'.");
    struct ASTNode *condition = parse_expression(parser);
    expect(parser, TOKEN_RIGHT_PAREN, "Expected ')' after if condition.");
    expect(parser, TOKEN_LEFT_BRACE, "Expected '{' after if condition.");
    struct ASTNode *body = parse_block(parser);
    expect(parser, TOKEN_RIGHT_BRACE, "Expected '}' after if body.");

    // Parse else block with support for "else if"
    struct ASTNode *else_body = NULL;
    if (match(parser, TOKEN_ELSE)) {
      advance(parser); // Consume 'else'
      if (match(parser, TOKEN_IF)) {
         // "else if" chain: recursively parse the if-statement
         else_body = parse_statement(parser);
      } else {
         // Regular else block: expect a block in braces
         expect(parser, TOKEN_LEFT_BRACE, "Expected '{' after else.");
         else_body = parse_block(parser);
         expect(parser, TOKEN_RIGHT_BRACE, "Expected '}' after else body.");
      }
    }

    // Create if statement node
    struct ASTNode *node = malloc(sizeof(struct ASTNode));
    node->type = NODE_IF_STATEMENT;
    node->if_stmt.condition = condition;
    node->if_stmt.body = body;
    node->if_stmt.else_body = else_body;
    node->next = NULL;
    return node;
  }

  // Variable declaration or expression statement
  if (match(parser, TOKEN_IDENTIFIER)) {
    struct Token *first_token = peek(parser);

    // Check if next token is an identifier (variable name)
    if (parser->position + 1 < parser->tokens->count &&
        parser->tokens->tokens[parser->position + 1].type == TOKEN_IDENTIFIER) {
      // Variable declaration
      advance(parser); // Consume datatype
      struct Token *datatype_token = first_token;
      char *datatype = strndup(&parser->input[datatype_token->start],
                             datatype_token->end - datatype_token->start);

      // Variable name
      expect(parser, TOKEN_IDENTIFIER, "Expected variable name.");
      struct Token *name_token = previous(parser);
      char *var_name = strndup(&parser->input[name_token->start],
                             name_token->end - name_token->start);

      // Match '='
      expect(parser, TOKEN_EQUAL, "Expected '=' after variable name.");

      // Parse expression
      struct ASTNode *value = parse_expression(parser);

      // Match ';'
      expect(parser, TOKEN_SEMICOLON,
             "Expected ';' after variable declaration.");

      // Create variable declaration node
      struct ASTNode *node = malloc(sizeof(struct ASTNode));
      node->type = NODE_VARIABLE_DECLARATION;
      node->var_decl.datatype = datatype;
      node->var_decl.name = var_name;
      node->var_decl.value = value;
      node->next = NULL;

      return node;
    } else if (parser->position + 1 < parser->tokens->count &&
              parser->tokens->tokens[parser->position + 1].type == TOKEN_EQUAL) {
      // Assignment statement
      advance(parser); // Consume identifier
      char *var_name = strndup(&parser->input[first_token->start],
                             first_token->end - first_token->start);

      // Match '='
      expect(parser, TOKEN_EQUAL, "Expected '=' after variable name.");

      // Parse expression
      struct ASTNode *value = parse_expression(parser);

      // Match ';'
      expect(parser, TOKEN_SEMICOLON, "Expected ';' after assignment.");

      // Create identifier node for target
      struct ASTNode *target = malloc(sizeof(struct ASTNode));
      target->type = NODE_IDENTIFIER;
      target->identifier.name = var_name;

      // Create assignment node
      struct ASTNode *node = malloc(sizeof(struct ASTNode));
      node->type = NODE_ASSIGNMENT;
      node->assignment.target = target;
      node->assignment.value = value;
      node->next = NULL;

      return node;
    }
  }

  // Return statement
  if (match(parser, TOKEN_RETURN)) {
    advance(parser); // Consume 'return'

    // Parse expression
    struct ASTNode *value = parse_expression(parser);

    // Match ';'
    expect(parser, TOKEN_SEMICOLON, "Expected ';' after return statement.");

    // Create return statement node
    struct ASTNode *node = malloc(sizeof(struct ASTNode));
    node->type = NODE_RETURN_STATEMENT;
    node->return_stmt.value = value;
    node->next = NULL;

    return node;
  }

  // Expression statement
  struct ASTNode *expr = parse_expression(parser);

  // Match ';'
  expect(parser, TOKEN_SEMICOLON, "Expected ';' after expression.");

  expr->next = NULL;
  return expr;
}

// Modified expression parsing with proper precedence
static struct ASTNode *parse_expression(struct Parser *parser) {
    return parse_equality(parser);
}

// Handles == and !=
static struct ASTNode *parse_equality(struct Parser *parser) {
    struct ASTNode *node = parse_additive(parser);

    while (match(parser, TOKEN_EQUAL_EQUAL) || match(parser, TOKEN_NOT_EQUAL)) {
        struct Token *op_token = advance(parser);
        char *operator = strndup(&parser->input[op_token->start], 
                               op_token->end - op_token->start);

        struct ASTNode *right = parse_additive(parser);

        struct ASTNode *bin_node = malloc(sizeof(struct ASTNode));
        bin_node->type = NODE_BINARY_OPERATION;
        bin_node->binary_op.operator = operator;
        bin_node->binary_op.left = node;
        bin_node->binary_op.right = right;
        bin_node->next = NULL;

        node = bin_node;
    }
    return node;
}

// Renamed from original parse_expression
static struct ASTNode *parse_additive(struct Parser *parser) {
    struct ASTNode *node = parse_term(parser);

    while (match(parser, TOKEN_PLUS) || match(parser, TOKEN_MINUS)) {
        struct Token *op_token = advance(parser);
        char operator = parser->input[op_token->start];

        struct ASTNode *right = parse_term(parser);

        struct ASTNode *bin_node = malloc(sizeof(struct ASTNode));
        bin_node->type = NODE_BINARY_OPERATION;
        bin_node->binary_op.operator = strndup(&operator, 1);
        bin_node->binary_op.left = node;
        bin_node->binary_op.right = right;
        bin_node->next = NULL;

        node = bin_node;
    }

    return node;
}

// Parse a term (handles *, /)
static struct ASTNode *parse_term(struct Parser *parser) {
    struct ASTNode *node = parse_factor(parser);

    while (match(parser, TOKEN_MULTIPLY) || match(parser, TOKEN_DIVIDE)) {
        struct Token *op_token = advance(parser);
        char operator = parser->input[op_token->start];

        struct ASTNode *right = parse_factor(parser);

        // Create binary operation node
        struct ASTNode *bin_node = malloc(sizeof(struct ASTNode));
        bin_node->type = NODE_BINARY_OPERATION;
        bin_node->binary_op.operator = strndup(&operator, 1);
        bin_node->binary_op.left = node;
        bin_node->binary_op.right = right;
        bin_node->next = NULL;

        node = bin_node;
    }

    return node;
}

// Parse a factor (primary expressions)
static struct ASTNode *parse_factor(struct Parser *parser) {
    return parse_primary(parser);
}

// Parse primary expressions
static struct ASTNode *parse_primary(struct Parser *parser) {
  if (match(parser, TOKEN_LITERAL_INT)) {
    // Integer literal
    struct Token *int_token = advance(parser);
    int len = int_token->end - int_token->start;
    char *value_str = strndup(&parser->input[int_token->start], len);
    int value = atoi(value_str);
    free(value_str);

    struct ASTNode *node = malloc(sizeof(struct ASTNode));
    node->type = NODE_INTEGER_LITERAL;
    node->int_literal.value = value;
    node->next = NULL;

    return node;
  } else if (match(parser, TOKEN_LITERAL_STRING)) {
    struct Token *str_token = advance(parser);
    int len = str_token->end - str_token->start;
    char *value = strndup(&parser->input[str_token->start], len);

    struct ASTNode *node = malloc(sizeof(struct ASTNode));
    node->type = NODE_STRING_LITERAL;
    node->string_literal.value = value;
    node->next = NULL;

    return node;
  } else if (match(parser, TOKEN_IDENTIFIER)) {
    struct Token *ident_token = advance(parser);
    int len = ident_token->end - ident_token->start;
    char *name = strndup(&parser->input[ident_token->start], len);

    // Check if function call
    if (match(parser, TOKEN_LEFT_PAREN)) {
      advance(parser); // Consume '('

      // Parse arguments
      struct ASTNode *arguments = NULL;
      if (!match(parser, TOKEN_RIGHT_PAREN)) {
        arguments = parse_arguments(parser);
      }

      expect(parser, TOKEN_RIGHT_PAREN,
             "Expected ')' after function arguments.");

      struct ASTNode *node = malloc(sizeof(struct ASTNode));
      node->type = NODE_FUNCTION_CALL;
      node->func_call.name = name;
      node->func_call.arguments = arguments;
      node->next = NULL;

      return node;
    } else {
      // Identifier
      struct ASTNode *node = malloc(sizeof(struct ASTNode));
      node->type = NODE_IDENTIFIER;
      node->identifier.name = name;
      node->next = NULL;

      return node;
    }
  } else {
    struct Token *current_token = peek(parser);
    fprintf(stderr, "Error on line %d: Unexpected token in primary expression.\n",
           current_token ? current_token->line : 0);
    exit(1);
  }
}

// Utility functions
static int match(struct Parser *parser, int token_type) {
  if (is_at_end(parser))
    return 0;
  return parser->tokens->tokens[parser->position].type == token_type;
}

static struct Token *advance(struct Parser *parser) {
  if (!is_at_end(parser))
    parser->position++;
  return previous(parser);
}

static struct Token *peek(struct Parser *parser) {
  if (is_at_end(parser))
    return NULL;
  return &parser->tokens->tokens[parser->position];
}

static struct Token *previous(struct Parser *parser) {
  return &parser->tokens->tokens[parser->position - 1];
}

static int is_at_end(struct Parser *parser) {
  return parser->position >= parser->tokens->count;
}

static void expect(struct Parser *parser, int token_type, const char *message) {
  if (match(parser, token_type)) {
    advance(parser);
  } else {
    struct Token *current = peek(parser);
    fprintf(stderr, "Error on line %d: %s\n", current ? current->line : 0, message);
    exit(1);
  }
}

void free_ast(struct ASTNode *node) {
  while (node) {
    struct ASTNode *next = node->next;

    if (node->type == NODE_FUNCTION_DECLARATION) {
      free(node->function_decl.name);
      free(node->function_decl.return_type);
      for (int i = 0; i < node->function_decl.param_count; i++) {
        free(node->function_decl.parameters[i].name);
        free(node->function_decl.parameters[i].type);
      }
      free(node->function_decl.parameters);
      free_ast(node->function_decl.body);
    } else if (node->type == NODE_VARIABLE_DECLARATION) {
      free(node->var_decl.datatype);
      free(node->var_decl.name);
      if (node->var_decl.value) {
        free_ast(node->var_decl.value);
      }
    } else if (node->type == NODE_BINARY_OPERATION) {
      free(node->binary_op.operator);
      free_ast(node->binary_op.left);
      free_ast(node->binary_op.right);
    } else if (node->type == NODE_IDENTIFIER) {
      free(node->identifier.name);
    } else if (node->type == NODE_FUNCTION_CALL) {
      free(node->func_call.name);
      free_ast(node->func_call.arguments);
    } else if (node->type == NODE_RETURN_STATEMENT) {
      if (node->return_stmt.value) {
        free_ast(node->return_stmt.value);
      }
    } else if (node->type == NODE_STRING_LITERAL) {
      free(node->string_literal.value);
    } else if (node->type == NODE_IF_STATEMENT) {
      free_ast(node->if_stmt.condition);
      free_ast(node->if_stmt.body);
      if (node->if_stmt.else_body) {
        free_ast(node->if_stmt.else_body);
      }
    }

    free(node);
    node = next;
  }
}

static struct ASTNode *parse_arguments(struct Parser *parser) {
  struct ASTNode *first_arg = parse_expression(parser);
  struct ASTNode *current = first_arg;

  while (match(parser, TOKEN_COMMA)) {
    advance(parser); // Consume comma
    struct ASTNode *next_arg = parse_expression(parser);
    current->next = next_arg;
    current = next_arg;
  }

  return first_arg;
}

static struct ASTNode *parse_block(struct Parser *parser) {
    struct ASTNode *body = NULL;
    struct ASTNode **current = &body;

    while (!is_at_end(parser) && !match(parser, TOKEN_RIGHT_BRACE)) {
        struct ASTNode *stmt = parse_statement(parser);
        if (stmt) {
            *current = stmt;
            current = &((*current)->next);
        } else {
            struct Token *current_token = peek(parser);
            fprintf(stderr, "Error on line %d: Invalid statement in block.\n",
                   current_token ? current_token->line : 0);
            exit(1);
        }
    }

    return body;
}
