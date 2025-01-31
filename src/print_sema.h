#include <stdio.h>
#include "common.h"

// Add forward declaration at the top
void print_symbol_table(struct SymbolTable *table, int indent);

// Print a single symbol's information
void print_symbol(struct Symbol *symbol, int indent) {
    for (int i = 0; i < indent; i++) printf("  ");

    printf("%s: ", symbol->name);
    if (symbol->type == SYMBOL_VARIABLE) {
        printf("Variable (type: %s, offset: %d, size: %d)\n",
               symbol->variable.data_type,
               symbol->variable.offset,
               symbol->variable.size);
    } else if (symbol->type == SYMBOL_FUNCTION) {
        printf("Function (return type: %s)\n", symbol->function.return_type);

        // Print parameters
        for (int i = 0; i < symbol->function.param_count; i++) {
            for (int j = 0; j < indent + 1; j++) printf("  ");
            printf("Parameter %d: %s\n", i, symbol->function.param_types[i]);
        }

        // Print local variables if any
        if (symbol->function.locals && symbol->function.locals->count > 0) {
            for (int j = 0; j < indent + 1; j++) printf("  ");
            printf("Local variables:\n");
            print_symbol_table(symbol->function.locals, indent + 2);
        }

        for (int j = 0; j < indent + 1; j++) printf("  ");
        printf("Stack size: %d\n", symbol->function.stack_size);
    }
}

// Print an entire symbol table
void print_symbol_table(struct SymbolTable *table, int indent) {
    if (!table) return;

    for (int i = 0; i < table->count; i++) {
        print_symbol(table->symbols[i], indent);
    }
}

// Print the entire semantic analysis context
void print_semantic_context(struct SemanticContext *context) {
    if (!context) {
        printf("No semantic context available\n");
        return;
    }

    printf("Semantic Analysis Results:\n");
    printf("Global scope:\n");
    print_symbol_table(context->global_scope, 1);

    if (context->had_error) {
        printf("\nSemantic analysis encountered errors\n");
    }
}
