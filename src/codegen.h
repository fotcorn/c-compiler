#pragma once

#include "common.h"
#include "sema.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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

// Operand types
#define OPERAND_REGISTER 1
#define OPERAND_IMMEDIATE 2
#define OPERAND_MEMORY 3
#define OPERAND_LABEL 4

// Common x86-64 registers
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

// Create a new assembly program
struct Assembly *create_assembly() {
    struct Assembly *assembly = malloc(sizeof(struct Assembly));
    assembly->sections = NULL;
    assembly->extern_symbols = NULL;
    assembly->extern_count = 0;
    assembly->string_literals = NULL;
    return assembly;
}

// Add an external symbol
void add_extern_symbol(struct Assembly *assembly, const char *symbol) {
    assembly->extern_count++;
    assembly->extern_symbols = realloc(assembly->extern_symbols,
                                     assembly->extern_count * sizeof(char*));
    assembly->extern_symbols[assembly->extern_count - 1] = strdup(symbol);
}

// Create a new section
struct Section *create_section(const char *name) {
    struct Section *section = malloc(sizeof(struct Section));
    section->name = strdup(name);
    section->instructions = NULL;
    section->next = NULL;
    return section;
}

// Add an instruction to a section
void add_instruction(struct Section *section, int type,
                    struct Operand dest, struct Operand src) {
    struct Instruction *instr = malloc(sizeof(struct Instruction));
    instr->type = type;
    instr->dest = dest;
    instr->src = src;
    instr->next = NULL;

    if (!section->instructions) {
        section->instructions = instr;
    } else {
        struct Instruction *last = section->instructions;
        while (last->next) {
            last = last->next;
        }
        last->next = instr;
    }
}

// Helper functions to create operands
struct Operand reg_operand(int reg) {
    struct Operand op = {.type = OPERAND_REGISTER, .reg = reg};
    return op;
}

struct Operand imm_operand(int value) {
    struct Operand op = {.type = OPERAND_IMMEDIATE, .immediate = value};
    return op;
}

struct Operand mem_operand(int base_reg, int offset) {
    struct Operand op = {.type = OPERAND_MEMORY};
    op.mem.base_reg = base_reg;
    op.mem.offset = offset;
    return op;
}

struct Operand label_operand(const char *label) {
    struct Operand op = {.type = OPERAND_LABEL};
    op.label = strdup(label);
    return op;
}

// Convert register number to string
const char *reg_to_str(int reg) {
    if (reg == REG_RAX) return "rax";
    if (reg == REG_RBX) return "rbx";
    if (reg == REG_RCX) return "rcx";
    if (reg == REG_RDX) return "rdx";
    if (reg == REG_RSP) return "rsp";
    if (reg == REG_RBP) return "rbp";
    if (reg == REG_RDI) return "rdi";
    if (reg == REG_RSI) return "rsi";
    if (reg == REG_R8)  return "r8";
    if (reg == REG_R9)  return "r9";
    return "unknown";
}

// Convert instruction type to string
const char *instr_to_str(int type) {
    if (type == INSTR_MOV) return "movq";
    if (type == INSTR_ADD) return "addq";
    if (type == INSTR_SUB) return "subq";
    if (type == INSTR_PUSH) return "pushq";
    if (type == INSTR_POP) return "popq";
    if (type == INSTR_CALL) return "call";
    if (type == INSTR_RET) return "ret";
    if (type == INSTR_LEA) return "leaq";
    if (type == INSTR_MUL) return "imulq";
    if (type == INSTR_DIV) return "idivq";
    return "unknown";
}

// Print an operand
void print_operand(FILE *out, struct Operand op) {
    if (op.type == OPERAND_REGISTER) {
        fprintf(out, "%%%s", reg_to_str(op.reg));
    } else if (op.type == OPERAND_IMMEDIATE) {
        fprintf(out, "$%d", op.immediate);
    } else if (op.type == OPERAND_MEMORY) {
        if (op.mem.offset) {
            fprintf(out, "%d(%%%s)", op.mem.offset, reg_to_str(op.mem.base_reg));
        } else {
            fprintf(out, "(%%%s)", reg_to_str(op.mem.base_reg));
        }
    } else if (op.type == OPERAND_LABEL) {
        // Check if it's an internal label (starts with .)
        if (op.label[0] == '.') {
            fprintf(out, "%s(%%rip)", op.label);
        } else {
            fprintf(out, "%s", op.label);
        }
    }
}

// Print an instruction
void print_instruction(FILE *out, struct Instruction *instr) {
    fprintf(out, "    %s ", instr_to_str(instr->type));

    // Special case for RET which has no operands
    if (instr->type == INSTR_RET) {
        fprintf(out, "\n");
        return;
    }

    // Special case for POP and PUSH which have only one operand
    if (instr->type == INSTR_POP || instr->type == INSTR_PUSH) {
        print_operand(out, instr->src);
        fprintf(out, "\n");
        return;
    }

    // Special case for CALL
    if (instr->type == INSTR_CALL) {
        print_operand(out, instr->src);
        fprintf(out, "\n");
        return;
    }

    // Special case for DIV which has only one operand
    if (instr->type == INSTR_DIV) {
        print_operand(out, instr->dest);
        fprintf(out, "\n");
        return;
    }

    // For most instructions, print source first, then destination (AT&T syntax)
    print_operand(out, instr->src);
    fprintf(out, ", ");
    print_operand(out, instr->dest);
    fprintf(out, "\n");
}

char *add_string_literal(struct Assembly *assembly, const char *value) {
    static int string_counter = 0;
    char label[32];
    snprintf(label, sizeof(label), ".LC%d", string_counter++);

    struct StringLiteral *str = malloc(sizeof(struct StringLiteral));
    str->label = strdup(label);
    str->value = strdup(value);
    str->next = assembly->string_literals;
    assembly->string_literals = str;

    return str->label;
}

// Print the complete assembly program
void print_assembly(FILE *out, struct Assembly *assembly) {
    // Print extern declarations
    for (int i = 0; i < assembly->extern_count; i++) {
        fprintf(out, ".extern %s\n", assembly->extern_symbols[i]);
    }
    fprintf(out, "\n");

    // Print data section with all string literals
    fprintf(out, ".section .data\n");
    struct StringLiteral *str = assembly->string_literals;
    while (str) {
        fprintf(out, "%s:\n", str->label);
        fprintf(out, "    .string %s\n", str->value);
        str = str->next;
    }
    fprintf(out, "\n");

    // Print text section
    fprintf(out, ".section .text\n");
    fprintf(out, ".globl main\n");
    fprintf(out, "main:\n");

    // Print each section
    struct Section *section = assembly->sections;
    while (section) {
        struct Instruction *instr = section->instructions;
        while (instr) {
            print_instruction(out, instr);
            instr = instr->next;
        }
        section = section->next;
    }
}

// Generate code for binary operation
void generate_binary_operation(struct Section *text, struct ASTNode *node, struct Symbol *func) {
    // First, save RDX as we'll need it for division
    add_instruction(text, INSTR_PUSH, reg_operand(REG_RDX), reg_operand(REG_RDX));

    // For nested binary operations on the right, evaluate them first
    if (node->binary_op.right->type == NODE_BINARY_OPERATION) {
        // Save RAX if we need it for the left operand
        if (node->binary_op.left->type == NODE_IDENTIFIER) {
            add_instruction(text, INSTR_PUSH, reg_operand(REG_RAX), reg_operand(REG_RAX));
        }
        
        // Generate code for right binary operation
        generate_binary_operation(text, node->binary_op.right, func);
        
        // Move result to RCX
        add_instruction(text, INSTR_MOV, reg_operand(REG_RCX), reg_operand(REG_RAX));
        
        // Restore RAX if we saved it
        if (node->binary_op.left->type == NODE_IDENTIFIER) {
            add_instruction(text, INSTR_POP, reg_operand(REG_RAX), reg_operand(REG_RAX));
        }
    }

    // Generate code for left operand
    if (node->binary_op.left->type == NODE_IDENTIFIER) {
        struct Symbol *left_var = lookup_symbol(func->function.locals,
                                              node->binary_op.left->identifier.name);
        if (left_var) {
            add_instruction(text, INSTR_MOV, reg_operand(REG_RAX),
                          mem_operand(REG_RBP, left_var->variable.offset));
        }
    } else if (node->binary_op.left->type == NODE_INTEGER_LITERAL) {
        add_instruction(text, INSTR_MOV, reg_operand(REG_RAX),
                       imm_operand(node->binary_op.left->int_literal.value));
    } else if (node->binary_op.left->type == NODE_BINARY_OPERATION) {
        generate_binary_operation(text, node->binary_op.left, func);
    }

    // For division, we need to save the left operand
    if (strcmp(node->binary_op.operator, "/") == 0) {
        add_instruction(text, INSTR_PUSH, reg_operand(REG_RAX), reg_operand(REG_RAX));
    }

    // Generate code for right operand (if not already handled)
    if (node->binary_op.right->type != NODE_BINARY_OPERATION) {
        if (node->binary_op.right->type == NODE_IDENTIFIER) {
            struct Symbol *right_var = lookup_symbol(func->function.locals,
                                                   node->binary_op.right->identifier.name);
            if (right_var) {
                if (strcmp(node->binary_op.operator, "/") == 0) {
                    add_instruction(text, INSTR_MOV, reg_operand(REG_RBX),
                                  mem_operand(REG_RBP, right_var->variable.offset));
                } else {
                    add_instruction(text, INSTR_MOV, reg_operand(REG_RCX),
                                  mem_operand(REG_RBP, right_var->variable.offset));
                }
            }
        } else if (node->binary_op.right->type == NODE_INTEGER_LITERAL) {
            if (strcmp(node->binary_op.operator, "/") == 0) {
                add_instruction(text, INSTR_MOV, reg_operand(REG_RBX),
                              imm_operand(node->binary_op.right->int_literal.value));
            } else {
                add_instruction(text, INSTR_MOV, reg_operand(REG_RCX),
                              imm_operand(node->binary_op.right->int_literal.value));
            }
        }
    }

    // For division, restore left operand to RAX
    if (strcmp(node->binary_op.operator, "/") == 0) {
        add_instruction(text, INSTR_POP, reg_operand(REG_RAX), reg_operand(REG_RAX));
    }

    // Perform the operation
    if (strcmp(node->binary_op.operator, "+") == 0) {
        add_instruction(text, INSTR_ADD, reg_operand(REG_RAX), reg_operand(REG_RCX));
    } else if (strcmp(node->binary_op.operator, "-") == 0) {
        add_instruction(text, INSTR_SUB, reg_operand(REG_RAX), reg_operand(REG_RCX));
    } else if (strcmp(node->binary_op.operator, "*") == 0) {
        add_instruction(text, INSTR_MUL, reg_operand(REG_RAX), reg_operand(REG_RCX));
    } else if (strcmp(node->binary_op.operator, "/") == 0) {
        // Clear RDX for division
        add_instruction(text, INSTR_MOV, reg_operand(REG_RDX), imm_operand(0));
        // Perform division: RAX = RDX:RAX / RBX
        struct Operand no_op = {0};  // Empty operand for single-operand instructions
        add_instruction(text, INSTR_DIV, reg_operand(REG_RBX), no_op);
    }

    // Restore RDX
    add_instruction(text, INSTR_POP, reg_operand(REG_RDX), reg_operand(REG_RDX));
}

// Generate code for assignment
void generate_assignment(struct Section *text, struct ASTNode *node, struct Symbol *func) {
    // Generate code for right-hand side
    if (node->assignment.value->type == NODE_INTEGER_LITERAL) {
        add_instruction(text, INSTR_MOV, reg_operand(REG_RAX),
                      imm_operand(node->assignment.value->int_literal.value));
    } else if (node->assignment.value->type == NODE_BINARY_OPERATION) {
        generate_binary_operation(text, node->assignment.value, func);
    } else if (node->assignment.value->type == NODE_IDENTIFIER) {
        struct Symbol *right_var = lookup_symbol(func->function.locals,
                                               node->assignment.value->identifier.name);
        if (right_var) {
            add_instruction(text, INSTR_MOV, reg_operand(REG_RAX),
                          mem_operand(REG_RBP, right_var->variable.offset));
        }
    }

    // Store result in target variable
    if (node->assignment.target->type == NODE_IDENTIFIER) {
        struct Symbol *var = lookup_symbol(func->function.locals,
                                         node->assignment.target->identifier.name);
        if (var) {
            add_instruction(text, INSTR_MOV,
                          mem_operand(REG_RBP, var->variable.offset),
                          reg_operand(REG_RAX));
        }
    }
}

// Main code generation function
struct Assembly *generate_code(struct ASTNode *ast, struct SemanticContext *context) {
    struct Assembly *assembly = create_assembly();

    // Add printf as external symbol
    add_extern_symbol(assembly, "printf");

    // Create text section
    struct Section *text = create_section(".text");
    text->next = NULL;
    assembly->sections = text;

    // Generate code for the AST
    while (ast) {
        if (ast->type == NODE_FUNCTION_DECLARATION) {
            // Get function's symbol table and stack size
            struct Symbol *func = lookup_symbol(context->global_scope, ast->function_decl.name);
            if (!func) continue;  // Should never happen after semantic analysis

            // Function prologue
            add_instruction(text, INSTR_PUSH, reg_operand(REG_RBP), reg_operand(REG_RBP));
            add_instruction(text, INSTR_MOV, reg_operand(REG_RBP), reg_operand(REG_RSP));

            // Reserve stack space for all variables
            if (func->function.stack_size > 0) {
                add_instruction(text, INSTR_SUB, reg_operand(REG_RSP),
                              imm_operand(func->function.stack_size));
            }

            // Generate code for function body
            struct ASTNode *body = ast->function_decl.body;
            while (body) {
                if (body->type == NODE_VARIABLE_DECLARATION) {
                    if (body->var_decl.value) {
                        struct Symbol *var = lookup_symbol(func->function.locals, body->var_decl.name);
                        if (var) {
                            if (body->var_decl.value->type == NODE_INTEGER_LITERAL) {
                                add_instruction(text, INSTR_MOV,
                                             mem_operand(REG_RBP, var->variable.offset),
                                             imm_operand(body->var_decl.value->int_literal.value));
                            } else if (body->var_decl.value->type == NODE_BINARY_OPERATION) {
                                generate_binary_operation(text, body->var_decl.value, func);
                                add_instruction(text, INSTR_MOV,
                                             mem_operand(REG_RBP, var->variable.offset),
                                             reg_operand(REG_RAX));
                            }
                        }
                    }
                } else if (body->type == NODE_BINARY_OPERATION) {
                    generate_binary_operation(text, body, func);
                } else if (body->type == NODE_ASSIGNMENT) {
                    generate_assignment(text, body, func);
                } else if (body->type == NODE_FUNCTION_CALL) {
                    if (strcmp(body->func_call.name, "printf") == 0) {
                        // Save registers we need to preserve
                        add_instruction(text, INSTR_PUSH, reg_operand(REG_RDI), reg_operand(REG_RDI));
                        add_instruction(text, INSTR_PUSH, reg_operand(REG_RSI), reg_operand(REG_RSI));
                        add_instruction(text, INSTR_PUSH, reg_operand(REG_RDX), reg_operand(REG_RDX));
                        add_instruction(text, INSTR_PUSH, reg_operand(REG_RCX), reg_operand(REG_RCX));
                        add_instruction(text, INSTR_PUSH, reg_operand(REG_R8), reg_operand(REG_R8));
                        add_instruction(text, INSTR_PUSH, reg_operand(REG_R9), reg_operand(REG_R9));

                        // Process format string
                        struct ASTNode *arg = body->func_call.arguments;
                        if (arg && arg->type == NODE_STRING_LITERAL) {
                            char *label = add_string_literal(assembly, arg->string_literal.value);
                            add_instruction(text, INSTR_LEA, reg_operand(REG_RDI),
                                         label_operand(label));
                            arg = arg->next;
                        }

                        // Process remaining arguments
                        int reg_args[] = {REG_RSI, REG_RDX, REG_RCX, REG_R8, REG_R9};
                        int arg_count = 0;

                        while (arg && arg_count < 6) {  // x86_64 can pass up to 6 args in registers
                            if (arg->type == NODE_IDENTIFIER) {
                                struct Symbol *var = lookup_symbol(func->function.locals,
                                                               arg->identifier.name);
                                if (var) {
                                    add_instruction(text, INSTR_MOV, reg_operand(reg_args[arg_count]),
                                                 mem_operand(REG_RBP, var->variable.offset));
                                }
                            } else if (arg->type == NODE_INTEGER_LITERAL) {
                                add_instruction(text, INSTR_MOV, reg_operand(reg_args[arg_count]),
                                             imm_operand(arg->int_literal.value));
                            }
                            arg_count++;
                            arg = arg->next;
                        }

                        // Clear AL register (needed for variadic functions)
                        add_instruction(text, INSTR_MOV, reg_operand(REG_RAX), imm_operand(0));

                        // Call printf
                        add_instruction(text, INSTR_CALL, label_operand("printf"),
                                     label_operand("printf"));

                        // Restore saved registers in reverse order
                        add_instruction(text, INSTR_POP, reg_operand(REG_R9), reg_operand(REG_R9));
                        add_instruction(text, INSTR_POP, reg_operand(REG_R8), reg_operand(REG_R8));
                        add_instruction(text, INSTR_POP, reg_operand(REG_RCX), reg_operand(REG_RCX));
                        add_instruction(text, INSTR_POP, reg_operand(REG_RDX), reg_operand(REG_RDX));
                        add_instruction(text, INSTR_POP, reg_operand(REG_RSI), reg_operand(REG_RSI));
                        add_instruction(text, INSTR_POP, reg_operand(REG_RDI), reg_operand(REG_RDI));
                    }
                } else if (body->type == NODE_RETURN_STATEMENT) {
                    if (body->return_stmt.value->type == NODE_INTEGER_LITERAL) {
                        add_instruction(text, INSTR_MOV, reg_operand(REG_RAX),
                                     imm_operand(body->return_stmt.value->int_literal.value));
                    }
                }
                body = body->next;
            }

            // Function epilogue
            add_instruction(text, INSTR_MOV, reg_operand(REG_RSP), reg_operand(REG_RBP));
            add_instruction(text, INSTR_POP, reg_operand(REG_RBP), reg_operand(REG_RBP));
            add_instruction(text, INSTR_RET, reg_operand(REG_RAX), reg_operand(REG_RAX));
        }
        ast = ast->next;
    }

    return assembly;
}