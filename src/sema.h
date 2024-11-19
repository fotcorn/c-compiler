#pragma once

#include "parser.h"

enum SymbolType {
    SYMBOL_VARIABLE,
    SYMBOL_FUNCTION,
    SYMBOL_STRUCT
};

struct SymbolTable;

// Symbol information
struct Symbol {
    char *name;
    enum SymbolType type;
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

struct SymbolTable *create_global_scope() {
    struct SymbolTable *scope = (struct SymbolTable *)malloc(sizeof(struct SymbolTable));
    scope->symbols = NULL;
    scope->count = 0;
    scope->capacity = 0;
    scope->parent = NULL;
    return scope;
}

struct SemanticContext {
    struct SymbolTable *current_scope;
    char *current_function;  // Name of function being analyzed
    int had_error;
};

// Main entry point for semantic analysis
int analyze_program(struct ASTNode *ast) {
    struct SemanticContext context = {
        .current_scope = create_global_scope(),
        .current_function = NULL,
        .had_error = 0
    };
    
    analyze_node(ast, &context);
    return context.had_error ? -1 : 0;
}

// Analyze a single node
void analyze_node(struct ASTNode *node, struct SemanticContext *context) {
    if (node->type == NODE_FUNCTION_DECLARATION) {
        analyze_function_declaration(node, context);
    } else if (node->type == NODE_VARIABLE_DECLARATION) {
        analyze_variable_declaration(node, context);
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
