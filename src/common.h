#pragma once

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
    int line;
};

struct TokenArray {
    struct Token *tokens;
    int capacity;
    int count;
};

// AST node types
int NODE_PROGRAM = 1;
int NODE_FUNCTION_DECLARATION = 2;
int NODE_VARIABLE_DECLARATION = 3;
int NODE_BINARY_OPERATION = 4;
int NODE_INTEGER_LITERAL = 5;
int NODE_IDENTIFIER = 6;
int NODE_FUNCTION_CALL = 7;
int NODE_RETURN_STATEMENT = 8;
int NODE_STRING_LITERAL = 9;

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
