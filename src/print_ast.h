#include <stdio.h>

#include "common.h"

static void print_indent(int level) {
    for (int i = 0; i < level; i++)
        printf("  ");
}

void print_ast(struct ASTNode *node, int indent) {
    while (node) {
        print_indent(indent);
        
        if (node->type == NODE_FUNCTION_DECLARATION) {
            printf("FunctionDeclaration: %s\n", node->function_decl.name);
            print_ast(node->function_decl.body, indent + 1);
        } else if (node->type == NODE_VARIABLE_DECLARATION) {
            printf("VariableDeclaration: %s %s\n", node->var_decl.datatype,
                  node->var_decl.name);
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
            if (node->func_call.arguments) {
                print_indent(indent + 1);
                printf("Arguments:\n");
                print_ast(node->func_call.arguments, indent + 2);
            }
        } else if (node->type == NODE_RETURN_STATEMENT) {
            printf("ReturnStatement\n");
            if (node->return_stmt.value) {
                print_ast(node->return_stmt.value, indent + 1);
            }
        } else if (node->type == NODE_STRING_LITERAL) {
            printf("StringLiteral: %s\n", node->string_literal.value);
        } else if (node->type == NODE_IF_STATEMENT) {
            printf("IfStatement\n");
            print_indent(indent + 1);
            printf("Condition:\n");
            print_ast(node->if_stmt.condition, indent + 2);
            print_indent(indent + 1);
            printf("Body:\n");
            print_ast(node->if_stmt.body, indent + 2);
            if (node->if_stmt.else_body) {
                print_indent(indent + 1);
                printf("Else Body:\n");
                print_ast(node->if_stmt.else_body, indent + 2);
            }
        } else {
            printf("Unknown node type\n");
        }
        node = node->next;
    }
}
