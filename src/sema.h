#include "common.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Forward declarations of semantic analysis functions
void analyze_node(struct ASTNode *node, struct SemanticContext *context);
void analyze_function_declaration(struct ASTNode *node, struct SemanticContext *context);
void analyze_variable_declaration(struct ASTNode *node, struct SemanticContext *context);
void analyze_expression(struct ASTNode *node, struct SemanticContext *context);

// Create a new symbol table
struct SymbolTable *create_symbol_table(void) {
    struct SymbolTable *table = malloc(sizeof(struct SymbolTable));
    table->symbols = NULL;
    table->count = 0;
    table->capacity = 0;
    table->parent = NULL;
    return table;
}

// Add a symbol to the symbol table
void add_symbol(struct SymbolTable *table, struct Symbol *symbol) {
    if (table->count >= table->capacity) {
        int new_capacity = table->capacity == 0 ? 8 : table->capacity * 2;
        table->symbols = realloc(table->symbols, new_capacity * sizeof(struct Symbol*));
        table->capacity = new_capacity;
    }
    table->symbols[table->count++] = symbol;
}

// Look up a symbol in the current scope and parent scopes
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

// Get size of a data type
int get_type_size(const char *type) {
    if (strcmp(type, "int") == 0) {
        return 8;  // Using 64-bit integers
    } else if (strcmp(type, "char") == 0) {
        return 1;
    }
    return 0;  // Unknown type
}

// Create a new variable symbol
struct Symbol *create_variable_symbol(const char *name, const char *type, int offset) {
    struct Symbol *sym = malloc(sizeof(struct Symbol));
    sym->name = strdup(name);
    sym->type = SYMBOL_VARIABLE;
    sym->variable.data_type = strdup(type);
    sym->variable.size = get_type_size(type);
    sym->variable.offset = offset;
    sym->scope = NULL;
    return sym;
}

// Create a new function symbol
struct Symbol *create_function_symbol(const char *name, const char *return_type,
                                    struct ASTNode *node) {
    struct Symbol *sym = malloc(sizeof(struct Symbol));
    sym->name = strdup(name);
    sym->type = SYMBOL_FUNCTION;
    sym->function.return_type = strdup(return_type);
    sym->function.param_count = node->function_decl.param_count;
    sym->function.param_types = malloc(node->function_decl.param_count * sizeof(char*));
    for (int i = 0; i < node->function_decl.param_count; i++) {
        sym->function.param_types[i] = strdup(node->function_decl.parameters[i].type);
    }
    sym->function.stack_size = 0;
    sym->function.locals = create_symbol_table();
    sym->scope = sym->function.locals;
    return sym;
}

// Main semantic analysis function
struct SemanticContext *analyze_program(struct ASTNode *ast) {
    struct SemanticContext *context = malloc(sizeof(struct SemanticContext));
    context->global_scope = create_symbol_table();
    context->current_scope = context->global_scope;
    context->current_function = NULL;
    context->had_error = 0;
    context->current_stack_offset = 0;

    analyze_node(ast, context);

    // Verify that main function exists
    if (!lookup_symbol(context->global_scope, "main")) {
        fprintf(stderr, "Error: No main function found\n");
        context->had_error = 1;
    }

    return context->had_error ? NULL : context;
}

// Analyze a single node
void analyze_node(struct ASTNode *node, struct SemanticContext *context) {
    if (!node) return;

    if (node->type == NODE_FUNCTION_DECLARATION) {
        analyze_function_declaration(node, context);
        analyze_node(node->next, context);
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

// Analyze a function declaration
void analyze_function_declaration(struct ASTNode *node, struct SemanticContext *context) {
    struct Symbol *func_sym = create_function_symbol(node->function_decl.name,
                                                   node->function_decl.return_type,
                                                   node);

    // Add function to current scope
    if (lookup_symbol(context->current_scope, func_sym->name)) {
        fprintf(stderr, "Error: Function %s already declared\n", func_sym->name);
        context->had_error = 1;
        return;
    }
    add_symbol(context->current_scope, func_sym);

    // Set up function context
    char *prev_function = context->current_function;
    context->current_function = func_sym->name;
    struct SymbolTable *prev_scope = context->current_scope;
    context->current_scope = func_sym->function.locals;
    context->current_scope->parent = prev_scope;
    context->current_stack_offset = 0;

    // Add parameters to function's local scope
    for (int i = 0; i < node->function_decl.param_count; i++) {
        // Parameters are stored in negative offsets like other locals
        context->current_stack_offset -= 8;
        struct Symbol *param_sym = create_variable_symbol(
            node->function_decl.parameters[i].name,
            node->function_decl.parameters[i].type,
            context->current_stack_offset
        );
        add_symbol(context->current_scope, param_sym);
    }

    // Analyze function body
    struct ASTNode *body = node->function_decl.body;
    while (body) {
        analyze_node(body, context);
        body = body->next;
    }

    // Update function's stack size (align to 16 bytes)
    func_sym->function.stack_size = (-context->current_stack_offset + 15) & ~15;

    // Restore context
    context->current_scope = prev_scope;
    context->current_function = prev_function;
}

// Analyze a variable declaration
void analyze_variable_declaration(struct ASTNode *node, struct SemanticContext *context) {
    // Allocate stack space for the variable
    context->current_stack_offset -= 8;  // 8 bytes for all variables for now

    struct Symbol *var_sym = create_variable_symbol(
        node->var_decl.name,
        node->var_decl.datatype,
        context->current_stack_offset
    );

    // Store the offset in the AST node for code generation
    node->var_decl.stack_offset = context->current_stack_offset;

    // Check if variable already exists in current scope
    if (lookup_symbol(context->current_scope, var_sym->name)) {
        fprintf(stderr, "Error: Variable %s already declared in current scope\n", var_sym->name);
        context->had_error = 1;
        return;
    }

    // Analyze initialization expression if present
    if (node->var_decl.value) {
        analyze_expression(node->var_decl.value, context);
    }

    add_symbol(context->current_scope, var_sym);
}

// Analyze an expression
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
            fprintf(stderr, "Error: Undefined variable %s\n", node->identifier.name);
            context->had_error = 1;
        }
    } else if (node->type == NODE_FUNCTION_CALL) {
        struct Symbol *sym = lookup_symbol(context->current_scope, node->func_call.name);
        if (!sym && strcmp(node->func_call.name, "printf") != 0) {
            fprintf(stderr, "Error: Undefined function %s\n", node->func_call.name);
            context->had_error = 1;
        }

        // Analyze function arguments
        struct ASTNode *arg = node->func_call.arguments;
        while (arg) {
            analyze_expression(arg, context);
            arg = arg->next;
        }
    }
}

// Get the symbol table for a function
struct SymbolTable *get_function_scope(struct SemanticContext *context, const char *func_name) {
    struct Symbol *func = lookup_symbol(context->global_scope, func_name);
    return func ? func->function.locals : NULL;
}

// Get a function's total stack size
int get_function_stack_size(struct SemanticContext *context, const char *func_name) {
    struct Symbol *func = lookup_symbol(context->global_scope, func_name);
    return func ? func->function.stack_size : 0;
}
