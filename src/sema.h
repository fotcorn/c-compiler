#pragma once

#include "common.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

// Forward declarations from parser.h
struct ASTNode;
void free_ast(struct ASTNode *node);

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
            int offset;      // Stack offset or struct field offset
            int size;        // Size in bytes
        } variable;
        
        struct {
            char *return_type;
            int param_count;
            char **param_types;
            int stack_size;  // Total stack frame size
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

struct SemanticContext {
    struct SymbolTable *current_scope;
    char *current_function;  // Name of function being analyzed
    int had_error;
};

// Forward declarations of all functions
struct SymbolTable *create_global_scope(void);
void analyze_node(struct ASTNode *node, struct SemanticContext *context);
void analyze_function_declaration(struct ASTNode *node, struct SemanticContext *context);
void analyze_variable_declaration(struct ASTNode *node, struct SemanticContext *context);
void analyze_expression(struct ASTNode *node, struct SemanticContext *context);
void add_symbol(struct SymbolTable *table, struct Symbol *symbol);
struct Symbol *lookup_symbol(struct SymbolTable *scope, const char *name);
int get_type_size(const char *type);

// Function implementations
struct SymbolTable *create_global_scope() {
    struct SymbolTable *scope = (struct SymbolTable *)malloc(sizeof(struct SymbolTable));
    scope->symbols = NULL;
    scope->count = 0;
    scope->capacity = 0;
    scope->parent = NULL;
    return scope;
}

int analyze_program(struct ASTNode *ast) {
    struct SemanticContext context = {
        .current_scope = create_global_scope(),
        .current_function = NULL,
        .had_error = 0
    };
    
    analyze_node(ast, &context);
    return context.had_error ? -1 : 0;
}

void analyze_node(struct ASTNode *node, struct SemanticContext *context) {
    if (!node) return;

    if (node->type == NODE_FUNCTION_DECLARATION) {
        analyze_function_declaration(node, context);
    } else if (node->type == NODE_VARIABLE_DECLARATION) {
        analyze_variable_declaration(node, context);
    } else if (node->type == NODE_RETURN_STATEMENT) {
        analyze_expression(node->return_stmt.value, context);
    } else if (node->type == NODE_FUNCTION_CALL || 
               node->type == NODE_BINARY_OPERATION ||
               node->type == NODE_INTEGER_LITERAL ||
               node->type == NODE_IDENTIFIER) {
        analyze_expression(node, context);
    }
}

void analyze_function_declaration(struct ASTNode *node, struct SemanticContext *context) {
    // Create symbol for function
    struct Symbol *func_sym = (struct Symbol *)malloc(sizeof(struct Symbol));
    func_sym->name = strdup(node->function_decl.name);
    func_sym->type = SYMBOL_FUNCTION;
    func_sym->scope = NULL;

    // Add function to current scope
    if (lookup_symbol(context->current_scope, func_sym->name)) {
        printf("Error: Function %s already declared\n", func_sym->name);
        context->had_error = 1;
        return;
    }
    add_symbol(context->current_scope, func_sym);

    // Create new scope for function body
    struct SymbolTable *function_scope = (struct SymbolTable *)malloc(sizeof(struct SymbolTable));
    function_scope->symbols = NULL;
    function_scope->count = 0;
    function_scope->capacity = 0;
    function_scope->parent = context->current_scope;
    func_sym->scope = function_scope;

    // Set current function name and scope
    char *prev_function = context->current_function;
    context->current_function = func_sym->name;
    struct SymbolTable *prev_scope = context->current_scope;
    context->current_scope = function_scope;

    // Analyze function body
    struct ASTNode *body = node->function_decl.body;
    while (body) {
        analyze_node(body, context);
        body = body->next;
    }

    // Restore previous scope and function name
    context->current_scope = prev_scope;
    context->current_function = prev_function;
}

void add_symbol(struct SymbolTable *table, struct Symbol *symbol) {
    if (table->count >= table->capacity) {
        int new_capacity = table->capacity == 0 ? 8 : table->capacity * 2;
        table->symbols = realloc(table->symbols, new_capacity * sizeof(struct Symbol*));
        table->capacity = new_capacity;
    }
    table->symbols[table->count++] = symbol;
}

struct Symbol *lookup_symbol(struct SymbolTable *scope, const char *name) {
    while (scope) {
        for (int i = 0; i < scope->count; i++) {
            if (strcmp(scope->symbols[i]->name, name) == 0) {
                return scope->symbols[i];
            }
        }
        scope = scope->parent;
    }
    return NULL;
}

int get_type_size(const char *type) {
    if (strcmp(type, "int") == 0) {
        return 4;
    } else if (strcmp(type, "char") == 0) {
        return 1;
    }
    return 0;  // Unknown type
}

void analyze_variable_declaration(struct ASTNode *node, struct SemanticContext *context) {
    struct Symbol *var_sym = (struct Symbol *)malloc(sizeof(struct Symbol));
    var_sym->name = strdup(node->var_decl.name);
    var_sym->type = SYMBOL_VARIABLE;
    var_sym->variable.data_type = strdup(node->var_decl.datatype);
    var_sym->variable.size = get_type_size(node->var_decl.datatype);
    var_sym->scope = NULL;

    // Check if variable already exists in current scope
    if (lookup_symbol(context->current_scope, var_sym->name)) {
        printf("Error: Variable %s already declared in current scope\n", var_sym->name);
        context->had_error = 1;
        return;
    }

    // Analyze initialization expression if present
    if (node->var_decl.value) {
        analyze_expression(node->var_decl.value, context);
    }

    add_symbol(context->current_scope, var_sym);
}

void analyze_expression(struct ASTNode *node, struct SemanticContext *context) {
    if (!node) return;

    if (node->type == NODE_BINARY_OPERATION) {
        analyze_expression(node->binary_op.left, context);
        analyze_expression(node->binary_op.right, context);
    } else if (node->type == NODE_INTEGER_LITERAL) {
        // Nothing to analyze for integer literals
    } else if (node->type == NODE_IDENTIFIER) {
        struct Symbol *sym = lookup_symbol(context->current_scope, node->identifier.name);
        if (!sym) {
            printf("Error: Undefined variable %s\n", node->identifier.name);
            context->had_error = 1;
        }
    } else if (node->type == NODE_FUNCTION_CALL) {
        struct Symbol *sym = lookup_symbol(context->current_scope, node->func_call.name);
        if (!sym) {
            // Special case for printf - it's a builtin function
            if (strcmp(node->func_call.name, "printf") != 0) {
                printf("Error: Undefined function %s\n", node->func_call.name);
                context->had_error = 1;
            }
        }
        
        // Analyze function arguments
        struct ASTNode *arg = node->func_call.arguments;
        while (arg) {
            analyze_expression(arg, context);
            arg = arg->next;
        }
    } else if (node->type == NODE_RETURN_STATEMENT) {
        if (node->return_stmt.value) {
            analyze_expression(node->return_stmt.value, context);
        }
    }
}

