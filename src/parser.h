#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "tokens.h"

// Add string storage
struct String {
    char *chars;
    int length;
};

// AST node types
enum NodeType {
    NODE_FUNCTION,
    NODE_BLOCK,
    NODE_VAR_DECL,
    NODE_ASSIGNMENT,
    NODE_BINARY_OP,
    NODE_INTEGER,
    NODE_IDENTIFIER
};

// AST node structure
struct ASTNode {
    enum NodeType type;
    struct ASTNode *left;
    struct ASTNode *right;
    int value;          // for integers
    struct String identifier;   // for variables and function names
    struct ASTNode *next;  // for statement lists in blocks
};

// Parser state
struct Parser {
    struct TokenArray *tokens;
    int current;
    const char *source;  // Add source text
};

// Forward declarations
struct ASTNode* parse_function(struct Parser *parser);
struct ASTNode* parse_block(struct Parser *parser);
struct ASTNode* parse_statement(struct Parser *parser);
struct ASTNode* parse_expression(struct Parser *parser);
struct ASTNode* parse_term(struct Parser *parser);
struct ASTNode* parse_factor(struct Parser *parser);

// Helper functions
static struct Token advance(struct Parser *parser) {
    struct Token token = parser->tokens->tokens[parser->current];
    parser->current++;
    return token;
}

static int match(struct Parser *parser, int type) {
    if (parser->current >= parser->tokens->count) return 0;
    if (parser->tokens->tokens[parser->current].type == type) {
        parser->current++;
        return 1;
    }
    return 0;
}

static int check(struct Parser *parser, int type) {
    if (parser->current >= parser->tokens->count) return 0;
    return parser->tokens->tokens[parser->current].type == type;
}

// Create a new AST node
static struct ASTNode* create_node(enum NodeType type) {
    struct ASTNode* node = (struct ASTNode*)malloc(sizeof(struct ASTNode));
    node->type = type;
    node->left = NULL;
    node->right = NULL;
    node->value = 0;
    node->identifier = (struct String){NULL, 0};
    node->next = NULL;
    return node;
}

// Create a new string
static struct String create_string(const char *start, int length) {
    char *chars = (char*)malloc(length + 1);
    memcpy(chars, start, length);
    chars[length] = '\0';
    return (struct String){chars, length};
}

// Free AST
static void free_ast(struct ASTNode *node) {
    if (node == NULL) return;
    
    free_ast(node->left);
    free_ast(node->right);
    free_ast(node->next);
    free(node->identifier.chars);
    free(node);
}

// Print indentation
static void print_indent(int level) {
    for (int i = 0; i < level; i++) {
        printf("  ");
    }
}

// Print AST recursively
static void print_ast(struct ASTNode *node, int level) {
    if (node == NULL) return;
    
    print_indent(level);
    
    switch (node->type) {
        case NODE_FUNCTION:
            printf("Function: %s\n", node->identifier.chars);
            print_ast(node->left, level + 1);  // print body
            break;
            
        case NODE_BLOCK:
            printf("Block:\n");
            struct ASTNode *stmt = node;
            while (stmt != NULL) {
                print_ast(stmt, level + 1);
                stmt = stmt->next;
            }
            break;
            
        case NODE_VAR_DECL:
            printf("VarDecl: %s\n", node->identifier.chars);
            break;
            
        case NODE_ASSIGNMENT:
            printf("Assign: %s =\n", node->identifier.chars);
            print_ast(node->right, level + 1);
            break;
            
        case NODE_BINARY_OP:
            printf("BinaryOp: ");
            if (node->value == TOKEN_PLUS) {
                printf("+\n");
            } else if (node->value == TOKEN_MINUS) {
                printf("-\n");
            } else if (node->value == TOKEN_MULTIPLY) {
                printf("*\n");
            } else if (node->value == TOKEN_DIVIDE) {
                printf("/\n");
            } else {
                printf("unknown\n");
            }
            print_ast(node->left, level + 1);
            print_ast(node->right, level + 1);
            break;
            
        case NODE_INTEGER:
            printf("Integer: %d\n", node->value);
            break;
            
        case NODE_IDENTIFIER:
            printf("Identifier: %s\n", node->identifier.chars);
            break;
            
        default:
            printf("Unknown node type: %d\n", node->type);
    }
}

// Main parse function
int parse(struct TokenArray *tokens, const char *source) {
    struct Parser parser = {tokens, 0, source};
    struct ASTNode* ast = parse_function(&parser);
    
    // Validate and print AST
    if (ast == NULL) {
        fprintf(stderr, "Parsing failed\n");
        return 1;
    }
    
    printf("\nAbstract Syntax Tree:\n");
    print_ast(ast, 0);
    printf("\n");
    
    free_ast(ast);
    return 0;
}

// Parse a function definition
struct ASTNode* parse_function(struct Parser *parser) {
    struct ASTNode* node = create_node(NODE_FUNCTION);
    
    // Parse identifier (function name)
    if (match(parser, TOKEN_IDENTIFIER)) {
        struct Token token = parser->tokens->tokens[parser->current - 1];
        node->identifier = create_string(
            parser->source + token.start,
            token.end - token.start
        );
        
        if (!match(parser, TOKEN_LEFT_PAREN)) {
            fprintf(stderr, "Expected '(' after function name\n");
            return NULL;
        }
        
        if (!match(parser, TOKEN_RIGHT_PAREN)) {
            fprintf(stderr, "Expected ')' after parameters\n");
            return NULL;
        }
        
        if (!match(parser, TOKEN_LEFT_BRACE)) {
            fprintf(stderr, "Expected '{' to begin function body\n");
            return NULL;
        }
        
        node->left = parse_block(parser);
        
        if (!match(parser, TOKEN_RIGHT_BRACE)) {
            fprintf(stderr, "Expected '}' to end function body\n");
            return NULL;
        }
        
        return node;
    }
    
    fprintf(stderr, "Expected function name\n");
    return NULL;
}

// Parse a block of statements
struct ASTNode* parse_block(struct Parser *parser) {
    struct ASTNode* first = NULL;
    struct ASTNode* current = NULL;
    
    while (!check(parser, TOKEN_RIGHT_BRACE)) {
        struct ASTNode* stmt = parse_statement(parser);
        if (stmt == NULL) {
            if (first) free_ast(first);
            return NULL;
        }
        
        if (first == NULL) {
            first = stmt;
            current = stmt;
        } else {
            current->next = stmt;
            current = stmt;
        }
    }
    
    return first;
}

// Parse a statement
struct ASTNode* parse_statement(struct Parser *parser) {
    // Variable declaration: int identifier;
    if (check(parser, TOKEN_IDENTIFIER)) {
        struct Token token = parser->tokens->tokens[parser->current];
        // Check if token text is "int"
        int len = token.end - token.start;
        if (len == 3 && strncmp(parser->source + token.start, "int", 3) == 0) {
            advance(parser); // consume 'int'
            
            struct ASTNode* node = create_node(NODE_VAR_DECL);
            
            if (!match(parser, TOKEN_IDENTIFIER)) {
                fprintf(stderr, "Expected variable name\n");
                free_ast(node);
                return NULL;
            }
            
            struct Token id_token = parser->tokens->tokens[parser->current - 1];
            node->identifier = create_string(
                parser->source + id_token.start, 
                id_token.end - id_token.start
            );
            
            if (!match(parser, TOKEN_SEMICOLON)) {
                fprintf(stderr, "Expected semicolon after variable declaration\n");
                free_ast(node);
                return NULL;
            }
            
            return node;
        }
    }
    
    // Assignment: identifier = expression;
    if (check(parser, TOKEN_IDENTIFIER)) {
        struct Token id_token = advance(parser);
        struct ASTNode* node = create_node(NODE_ASSIGNMENT);
        node->identifier = create_string(
            parser->source + id_token.start,
            id_token.end - id_token.start
        );
        
        if (!match(parser, TOKEN_EQUAL)) {
            fprintf(stderr, "Expected '=' in assignment\n");
            free_ast(node);
            return NULL;
        }
        
        node->right = parse_expression(parser);
        if (node->right == NULL) {
            free_ast(node);
            return NULL;
        }
        
        if (!match(parser, TOKEN_SEMICOLON)) {
            fprintf(stderr, "Expected semicolon after assignment\n");
            free_ast(node);
            return NULL;
        }
        
        return node;
    }
    
    // Expression statement
    struct ASTNode* expr = parse_expression(parser);
    if (expr == NULL) return NULL;
    
    if (!match(parser, TOKEN_SEMICOLON)) {
        fprintf(stderr, "Expected semicolon after expression\n");
        free_ast(expr);
        return NULL;
    }
    
    return expr;
}

// Parse an expression
struct ASTNode* parse_expression(struct Parser *parser) {
    struct ASTNode* left = parse_term(parser);
    if (left == NULL) return NULL;
    
    while (check(parser, TOKEN_PLUS) || check(parser, TOKEN_MINUS)) {
        struct ASTNode* node = create_node(NODE_BINARY_OP);
        node->value = advance(parser).type;
        node->left = left;
        
        node->right = parse_term(parser);
        if (node->right == NULL) {
            free_ast(node);
            return NULL;
        }
        
        left = node;
    }
    
    return left;
}

// Parse a term
struct ASTNode* parse_term(struct Parser *parser) {
    struct ASTNode* left = parse_factor(parser);
    if (left == NULL) return NULL;
    
    while (check(parser, TOKEN_MULTIPLY) || check(parser, TOKEN_DIVIDE)) {
        struct ASTNode* node = create_node(NODE_BINARY_OP);
        node->value = advance(parser).type;
        node->left = left;
        
        node->right = parse_factor(parser);
        if (node->right == NULL) {
            free_ast(node);
            return NULL;
        }
        
        left = node;
    }
    
    return left;
}

// Parse a factor
struct ASTNode* parse_factor(struct Parser *parser) {
    if (match(parser, TOKEN_LITERAL_INT)) {
        struct Token token = parser->tokens->tokens[parser->current - 1];
        struct ASTNode* node = create_node(NODE_INTEGER);
        
        // Convert token text to integer value
        char temp[32];
        int len = token.end - token.start;
        strncpy(temp, parser->source + token.start, len);
        temp[len] = '\0';
        node->value = atoi(temp);
        return node;
    }
    
    if (match(parser, TOKEN_IDENTIFIER)) {
        struct Token token = parser->tokens->tokens[parser->current - 1];
        struct ASTNode* node = create_node(NODE_IDENTIFIER);
        node->identifier = create_string(
            parser->source + token.start,
            token.end - token.start
        );
        return node;
    }
    
    if (match(parser, TOKEN_LEFT_PAREN)) {
        struct ASTNode* node = parse_expression(parser);
        if (!match(parser, TOKEN_RIGHT_PAREN)) {
            fprintf(stderr, "Expected ')'\n");
            free_ast(node);
            return NULL;
        }
        return node;
    }
    
    fprintf(stderr, "Unexpected token in factor\n");
    return NULL;
}
