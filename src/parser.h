#include "tokens.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// AST node types
int NODE_PROGRAM = 1;
int NODE_FUNCTION_DECLARATION = 2;
int NODE_VARIABLE_DECLARATION = 3;
int NODE_BINARY_OPERATION = 4;
int NODE_INTEGER_LITERAL = 5;
int NODE_IDENTIFIER = 6;
int NODE_FUNCTION_CALL = 7;
int NODE_RETURN_STATEMENT = 8;

// Forward declaration of ASTNode
struct ASTNode;

// AST node structure
struct ASTNode {
    int type;
    union {
        // Function declaration
        struct {
            char *name;
            struct ASTNode *body;
        } function_decl;

        // Variable declaration
        struct {
            char *datatype;
            char *name;
            struct ASTNode *value;
        } var_decl;

        // Binary operation
        struct {
            char *operator;
            struct ASTNode *left;
            struct ASTNode *right;
        } binary_op;

        // Integer literal
        struct {
            int value;
        } int_literal;

        // Identifier
        struct {
            char *name;
        } identifier;

        // Function call
        struct {
            char *name;
            struct ASTNode *arguments;
        } func_call;

        // Return statement
        struct {
            struct ASTNode *value;
        } return_stmt;
    };
    struct ASTNode *next; // For linked list of statements
};


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
static struct ASTNode *parse_factor(struct Parser *parser);
static struct ASTNode *parse_primary(struct Parser *parser);
static int match(struct Parser *parser, int token_type);
static struct Token *peek(struct Parser *parser);
static struct Token *previous(struct Parser *parser);
static struct Token *advance(struct Parser *parser);
static int is_at_end(struct Parser *parser);
static void expect(struct Parser *parser, int token_type, const char *message);

// Implement the parse function
struct ASTNode *parse(struct TokenArray *tokens, char* input) {
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
        } else {
            struct Token *current_token = peek(parser);
            printf("Error on line %d: Expected function declaration.\n", 
                   current_token ? current_token->line : 0);
            exit(1);
        }
    }

    return node;
}

// Parse a function declaration
static struct ASTNode *parse_function_declaration(struct Parser *parser) {
    // Match 'int' or other datatype
    if (!match(parser, TOKEN_IDENTIFIER)) {
        return NULL;
    }
    struct Token *type_token = advance(parser);

    // Extract the datatype
    char *datatype = strndup(&parser->input[type_token->start], type_token->end - type_token->start);

    // Match function name (e.g., 'main')
    expect(parser, TOKEN_IDENTIFIER, "Expected function name.");
    struct Token *name_token = previous(parser);
    char *func_name = strndup(&parser->input[name_token->start], name_token->end - name_token->start);

    // Match '(' ')'
    expect(parser, TOKEN_LEFT_PAREN, "Expected '(' after function name.");
    expect(parser, TOKEN_RIGHT_PAREN, "Expected ')' after '('.");

    // Match '{'
    expect(parser, TOKEN_LEFT_BRACE, "Expected '{' before function body.");

    // Parse function body
    struct ASTNode *body = NULL;
    struct ASTNode **current = &body;

    while (!is_at_end(parser) && !match(parser, TOKEN_RIGHT_BRACE)) {
        struct ASTNode *stmt = parse_statement(parser);
        if (stmt) {
            *current = stmt;
            current = &((*current)->next);
        } else {
            struct Token *current_token = peek(parser);
            printf("Error on line %d: Invalid statement in function body.\n",
                   current_token ? current_token->line : 0);
            exit(1);
        }
    }

    // Match '}'
    expect(parser, TOKEN_RIGHT_BRACE, "Expected '}' after function body.");

    // Create function declaration node
    struct ASTNode *node = malloc(sizeof(struct ASTNode));
    node->type = NODE_FUNCTION_DECLARATION;
    node->function_decl.name = func_name;
    node->function_decl.body = body;
    node->next = NULL;

    free(datatype); // In this simple parser, we don't use the datatype

    return node;
}

// Parse a statement
static struct ASTNode *parse_statement(struct Parser *parser) {
    // Variable declaration or expression statement
    if (match(parser, TOKEN_IDENTIFIER)) {
        struct Token *type_token = peek(parser);

        // Check if next token is an identifier (variable name)
        if (parser->position + 1 < parser->tokens->count &&
            parser->tokens->tokens[parser->position + 1].type == TOKEN_IDENTIFIER) {
            // Variable declaration
            advance(parser); // Consume datatype
            struct Token *datatype_token = type_token;
            char *datatype = strndup(&parser->input[datatype_token->start], datatype_token->end - datatype_token->start);

            // Variable name
            expect(parser, TOKEN_IDENTIFIER, "Expected variable name.");
            struct Token *name_token = previous(parser);
            char *var_name = strndup(&parser->input[name_token->start], name_token->end - name_token->start);

            // Match '='
            expect(parser, TOKEN_EQUAL, "Expected '=' after variable name.");

            // Parse expression
            struct ASTNode *value = parse_expression(parser);

            // Match ';'
            expect(parser, TOKEN_SEMICOLON, "Expected ';' after variable declaration.");

            // Create variable declaration node
            struct ASTNode *node = malloc(sizeof(struct ASTNode));
            node->type = NODE_VARIABLE_DECLARATION;
            node->var_decl.datatype = datatype;
            node->var_decl.name = var_name;
            node->var_decl.value = value;
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

// Parse an expression (handles binary operations)
static struct ASTNode *parse_expression(struct Parser *parser) {
    return parse_factor(parser);
}

// Parse factors (handles '+', '-' operators)
static struct ASTNode *parse_factor(struct Parser *parser) {
    struct ASTNode *node = parse_primary(parser);

    while (match(parser, TOKEN_PLUS) || match(parser, TOKEN_MINUS)) {
        struct Token *op_token = advance(parser);
        char operator = parser->input[op_token->start];

        struct ASTNode *right = parse_primary(parser);

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
    } else if (match(parser, TOKEN_IDENTIFIER)) {
        struct Token *ident_token = advance(parser);
        int len = ident_token->end - ident_token->start;
        char *name = strndup(&parser->input[ident_token->start], len);

        // Check if function call
        if (match(parser, TOKEN_LEFT_PAREN)) {
            advance(parser); // Consume '('

            // For simplicity, we assume no arguments in function calls
            expect(parser, TOKEN_RIGHT_PAREN, "Expected ')' after function call.");

            struct ASTNode *node = malloc(sizeof(struct ASTNode));
            node->type = NODE_FUNCTION_CALL;
            node->func_call.name = name;
            node->func_call.arguments = NULL;
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
        printf("Error on line %d: Unexpected token in primary expression.\n",
               current_token ? current_token->line : 0);
        exit(1);
    }
}

// Utility functions
static int match(struct Parser *parser, int token_type) {
    if (is_at_end(parser)) return 0;
    return parser->tokens->tokens[parser->position].type == token_type;
}

static struct Token *advance(struct Parser *parser) {
    if (!is_at_end(parser)) parser->position++;
    return previous(parser);
}

static struct Token *peek(struct Parser *parser) {
    if (is_at_end(parser)) return NULL;
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
        printf("Error on line %d: %s\n", current ? current->line : 0, message);
        exit(1);
    }
}

// Function to print the AST
void print_ast(struct ASTNode *node, int indent) {
    while (node) {
        for (int i = 0; i < indent; i++) printf("  ");
        if (node->type == NODE_FUNCTION_DECLARATION) {
            printf("FunctionDeclaration: %s\n", node->function_decl.name);
            print_ast(node->function_decl.body, indent + 1);
        } else if (node->type == NODE_VARIABLE_DECLARATION) {
            printf("VariableDeclaration: %s %s\n", node->var_decl.datatype, node->var_decl.name);
            if (node->var_decl.value) {
                print_ast(node->var_decl.value, indent + 1);
            }
        } else if (node->type == NODE_BINARY_OPERATION) {
            printf("BinaryOperation: %s\n", node->binary_op.operator);
            print_ast(node->binary_op.left, indent + 1);
            print_ast(node->binary_op.right, indent + 1);
        } else if (node->type == NODE_INTEGER_LITERAL) {
            printf("IntegerLiteral: %d\n", node->int_literal.value);
        } else if (node->type == NODE_IDENTIFIER) {
            printf("Identifier: %s\n", node->identifier.name);
        } else if (node->type == NODE_FUNCTION_CALL) {
            printf("FunctionCall: %s\n", node->func_call.name);
        } else if (node->type == NODE_RETURN_STATEMENT) {
            printf("ReturnStatement\n");
            if (node->return_stmt.value) {
                print_ast(node->return_stmt.value, indent + 1);
            }
        } else {
            printf("Unknown node type\n");
        }
        node = node->next;
    }
}

void free_ast(struct ASTNode *node) {
    while (node) {
        struct ASTNode *next = node->next;
        
        if (node->type == NODE_FUNCTION_DECLARATION) {
            free(node->function_decl.name);
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
        }
        
        free(node);
        node = next;
    }
}
