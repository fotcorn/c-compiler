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
#define NODE_IF_STATEMENT 11
#define NODE_WHILE_STATEMENT 12

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

    // If statement
    struct {
      struct ASTNode *condition;
      struct ASTNode *body;
      struct ASTNode *else_body;
    } if_stmt;

    // While statement
    struct {
      struct ASTNode *condition;
      struct ASTNode *body;
    } while_stmt;
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
    struct Operand op1;
    struct Operand op2; 
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

// Instruction types
#define INSTR_MOV 1
#define INSTR_ADD 2
#define INSTR_SUB 3
#define INSTR_PUSH 4
#define INSTR_POP 5
#define INSTR_CALL 6
#define INSTR_RET 7
#define INSTR_LEA 8
#define INSTR_MUL 9
#define INSTR_DIV 10
#define INSTR_LABEL 11
#define INSTR_CMP 12
#define INSTR_SET_EQ 13
#define INSTR_SET_NE 14
#define INSTR_MOVZX 15
#define INSTR_JE 16
#define INSTR_JMP 17

// Operand types
#define OPERAND_EMPTY 0       // For instructions with no operand
#define OPERAND_REGISTER 1
#define OPERAND_IMMEDIATE 2
#define OPERAND_MEMORY 3
#define OPERAND_LABEL 4
#define OPERAND_RIP_LABEL 5   // For RIP-relative labels

// All x86-64 64-bit registers
#define REG_RAX 1
#define REG_RBX 2
#define REG_RCX 3
#define REG_RDX 4
#define REG_RSP 5
#define REG_RBP 6
#define REG_RDI 7
#define REG_RSI 8
#define REG_R8  9
#define REG_R9  10
#define REG_R10 11
#define REG_R11 12
#define REG_R12 13
#define REG_R13 14
#define REG_R14 15
#define REG_R15 16
#define REG_AL 17

#define REG_COUNT 16
