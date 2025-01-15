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
#define NODE_ASSIGNMENT 10

// Forward declaration of ASTNode
struct ASTNode;

// Function parameter structure
struct FunctionParameter {
  char *name;
  char *type;
};

// AST node structure
struct ASTNode {
  int type;
  union {
    // Function declaration
    struct {
      char *name;
      char *return_type;
      struct FunctionParameter *parameters;
      int param_count;
      struct ASTNode *body;
    } function_decl;

    // Variable declaration
    struct {
      char *datatype;
      char *name;
      struct ASTNode *value;
      int stack_offset;
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

    // Assignment
    struct {
      struct ASTNode *target;
      struct ASTNode *value;
    } assignment;
  };
  struct ASTNode *next; // For linked list of statements
};

// Symbol types
#define SYMBOL_VARIABLE 1
#define SYMBOL_FUNCTION 2
#define SYMBOL_STRUCT 3

// Forward declaration of SymbolTable
struct SymbolTable;

// Symbol information
struct Symbol {
    char *name;
    int type;
    union {
        struct {
            char *data_type;
            int offset;      // Stack offset from RBP
            int size;       // Size in bytes
        } variable;

        struct {
            char *return_type;
            int param_count;
            char **param_types;
            int stack_size;  // Total stack frame size
            struct SymbolTable *locals;  // Local variables
        } function;

        struct {
            int total_size;  // Total struct size
            int field_count;
            struct Symbol **fields;  // Array of field symbols
        } struct_info;
    };
    struct SymbolTable *scope;  // Points to nested scope if this symbol creates one
};

// Symbol table for tracking scopes
struct SymbolTable {
    struct Symbol **symbols;
    int count;
    int capacity;
    struct SymbolTable *parent;  // Parent scope
};

// Semantic analysis context
struct SemanticContext {
    struct SymbolTable *current_scope;
    struct SymbolTable *global_scope;  // Keep track of global scope
    char *current_function;  // Name of function being analyzed
    int had_error;
    int current_stack_offset;  // Track current stack offset for variables
};

// Symbol table functions
struct Symbol *lookup_symbol(struct SymbolTable *scope, const char *name);

// Represents an operand in an assembly instruction
struct Operand {
    int type;
    union {
        int reg;            // For register operands
        int immediate;      // For immediate values
        struct {
            int base_reg;   // Base register for memory operands
            int offset;     // Offset for memory operands
        } mem;
        char *label;        // For labels and function names
    };
};

// Represents a single assembly instruction
struct Instruction {
    int type;
    struct Operand dest;
    struct Operand src;
    struct Instruction *next;
};

// Represents a section of assembly code
struct Section {
    char *name;
    struct Instruction *instructions;
    struct Section *next;
};

// String literals for the data section
struct StringLiteral {
    char *label;
    char *value;
    struct StringLiteral *next;
};

// Represents the complete assembly program
struct Assembly {
    struct Section *sections;
    char **extern_symbols;  // Array of external symbols (e.g., printf)
    int extern_count;
    struct StringLiteral *string_literals;
};
