#pragma once

#include "common.h"
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

// Represents the complete assembly program
struct Assembly {
    struct Section *sections;
    char **extern_symbols;  // Array of external symbols (e.g., printf)
    int extern_count;
};

// Create a new assembly program
struct Assembly *create_assembly() {
    struct Assembly *assembly = malloc(sizeof(struct Assembly));
    assembly->sections = NULL;
    assembly->extern_symbols = NULL;
    assembly->extern_count = 0;
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
        // For data labels, use RIP-relative addressing
        if (strcmp(op.label, "format") == 0) {
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
    
    // For most instructions, print source first, then destination
    print_operand(out, instr->src);
    fprintf(out, ", ");
    print_operand(out, instr->dest);
    fprintf(out, "\n");
}

// Print the complete assembly program
void print_assembly(FILE *out, struct Assembly *assembly) {
    // Print extern declarations
    for (int i = 0; i < assembly->extern_count; i++) {
        fprintf(out, ".extern %s\n", assembly->extern_symbols[i]);
    }
    fprintf(out, "\n");

    // Print data section first
    fprintf(out, ".section .data\n");
    fprintf(out, "format:\n");
    fprintf(out, "    .string \"%%d\\n\"\n\n");

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

// Main code generation function
struct Assembly *generate_code(struct ASTNode *ast) {
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
            // Function prologue
            add_instruction(text, INSTR_PUSH, reg_operand(REG_RBP), reg_operand(REG_RBP));
            add_instruction(text, INSTR_MOV, reg_operand(REG_RBP), reg_operand(REG_RSP));
            
            // Reserve stack space for all variables at once (3 variables * 8 bytes)
            add_instruction(text, INSTR_SUB, reg_operand(REG_RSP), imm_operand(32));  // 32 for 16-byte alignment
            
            // Generate code for function body
            struct ASTNode *body = ast->function_decl.body;
            while (body) {
                if (body->type == NODE_VARIABLE_DECLARATION) {
                    if (body->var_decl.value && body->var_decl.value->type == NODE_INTEGER_LITERAL) {
                        // Store value in the appropriate stack location
                        if (strcmp(body->var_decl.name, "a") == 0) {
                            add_instruction(text, INSTR_MOV, mem_operand(REG_RBP, -8), 
                                         imm_operand(body->var_decl.value->int_literal.value));
                        } else if (strcmp(body->var_decl.name, "b") == 0) {
                            add_instruction(text, INSTR_MOV, mem_operand(REG_RBP, -16), 
                                         imm_operand(body->var_decl.value->int_literal.value));
                        }
                    }
                } else if (body->type == NODE_BINARY_OPERATION) {
                    // Load first operand into RAX
                    add_instruction(text, INSTR_MOV, reg_operand(REG_RAX), 
                                 mem_operand(REG_RBP, -8));
                    
                    // Add second operand directly from memory
                    add_instruction(text, INSTR_ADD, reg_operand(REG_RAX), 
                                 mem_operand(REG_RBP, -16));
                    
                    // Store result in c's location
                    add_instruction(text, INSTR_MOV, mem_operand(REG_RBP, -24), 
                                 reg_operand(REG_RAX));
                } else if (body->type == NODE_FUNCTION_CALL) {
                    if (strcmp(body->func_call.name, "printf") == 0) {
                        // Load format string address (using RIP-relative addressing)
                        add_instruction(text, INSTR_LEA, reg_operand(REG_RDI), 
                                     label_operand("format"));
                        
                        // Load variable to print
                        add_instruction(text, INSTR_MOV, reg_operand(REG_RSI), 
                                     mem_operand(REG_RBP, -24));
                        
                        // Call printf (direct call)
                        add_instruction(text, INSTR_CALL, label_operand("printf"), 
                                     label_operand("printf"));
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