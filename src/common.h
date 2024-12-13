#pragma once

// Token types
#define TOKEN_LEFT_BRACE 1
#define TOKEN_RIGHT_BRACE 2
#define TOKEN_LEFT_PAREN 3
#define TOKEN_RIGHT_PAREN 4
#define TOKEN_LEFT_BRACKET 5
#define TOKEN_RIGHT_BRACKET 6
#define TOKEN_LITERAL_INT 7
#define TOKEN_LITERAL_CHAR 8
#define TOKEN_LITERAL_STRING 9
#define TOKEN_IDENTIFIER 10
#define TOKEN_RETURN 11
#define TOKEN_IF 12
#define TOKEN_ELSE 13
#define TOKEN_WHILE 14
#define TOKEN_STRUCT 15
#define TOKEN_SEMICOLON 16
#define TOKEN_COMMA 17
#define TOKEN_PLUS 18
#define TOKEN_MINUS 19
#define TOKEN_MULTIPLY 20
#define TOKEN_DIVIDE 21
#define TOKEN_EQUAL 22
#define TOKEN_EQUAL_EQUAL 23
#define TOKEN_NOT_EQUAL 24
#define TOKEN_LESS 25
#define TOKEN_LESS_EQUAL 26
#define TOKEN_GREATER 27
#define TOKEN_GREATER_EQUAL 28
#define TOKEN_PERIOD 29
#define TOKEN_LOGICAL_OR 30
#define TOKEN_LOGICAL_AND 31
#define TOKEN_AMPERSAND 32

struct Token {
    int type;
    int start;
    int end;
    int line;
};

struct TokenArray {
    struct Token *tokens;
    int capacity;
    int count;
};

// AST node types
#define NODE_PROGRAM 1
#define NODE_FUNCTION_DECLARATION 2
#define NODE_VARIABLE_DECLARATION 3
#define NODE_BINARY_OPERATION 4
#define NODE_INTEGER_LITERAL 5
#define NODE_IDENTIFIER 6
#define NODE_FUNCTION_CALL 7
#define NODE_RETURN_STATEMENT 8
#define NODE_STRING_LITERAL 9

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

        // String literal
        struct {
            char *value;
        } string_literal;
    };
    struct ASTNode *next; // For linked list of statements
};
