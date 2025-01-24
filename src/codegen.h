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
#define INSTR_MUL 9
#define INSTR_DIV 10
#define INSTR_LABEL 11
#define INSTR_CMP 12
#define INSTR_SET_EQ 13
#define INSTR_MOVZX 14

// Operand types
#define OPERAND_REGISTER 1
#define OPERAND_IMMEDIATE 2
#define OPERAND_MEMORY 3
#define OPERAND_LABEL 4

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

// All used temp regs are caller saved, which avoid needing to handle
// push/pop at the function level for callee saved registers.
// We also don't use RAX for simplicity.
static const int TEMP_REGS[] = {
    REG_R10,  // Never used for params, good first choice
    REG_R11,  // Never used for params
    REG_R9,   // Only used for 6th param (rare)
    REG_R8,   // Only used for 5th
    REG_RCX,  // 4th param
    REG_RDX,  // 3rd param
    REG_RSI,  // 2nd param
    REG_RDI   // 1st param (used most often for params)
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

// ---------------------- Register Allocation Context -----------------------
// We'll define a few more registers we can use as scratch registers for expressions.
// A small struct to keep track of which registers are currently in use.
// For simplicity, we store a small fixed array of possible scratch registers.
// "used[i] = 1" if that register is already allocated, 0 if free.
struct CodegenContext {
    int used[REG_COUNT];
};

// Initializes all registers as free.
static void init_codegen_context(struct CodegenContext *ctx) {
    for (int i = 0; i < REG_COUNT; i++) {
        ctx->used[i] = 0;
    }
}

// Marks a register as allocated, returns the register ID.
static int allocate_register(struct CodegenContext *ctx) {
    size_t i = 0;
    while (i < sizeof(TEMP_REGS) / sizeof(TEMP_REGS[0])) {
        int reg = TEMP_REGS[i];
        if (!ctx->used[reg - 1]) {
            ctx->used[reg - 1] = 1;
            return reg;
        }
        i++;
    }
    // Fallback if we run out of registers:
    fprintf(stderr, "Ran out of registers for expression.\n");
    exit(1);
}

// Frees a register so it can be used again.
static void free_register(struct CodegenContext *ctx, int reg) {
    ctx->used[reg - 1] = 0;
}

// --------------------- Unified Expression Generation -------------------------------
// Instead of special per-node code in each place, we define one function
// that recursively generates code for any expression and returns the reg
// holding the final result.

static int generate_expression(struct Section *text,
                               struct ASTNode *node,
                               struct Symbol *func,
                               struct Assembly *assembly,
                               struct CodegenContext *ctx);

// Implementation of generate_expression:
static int generate_expression(struct Section *text,
                               struct ASTNode *node,
                               struct Symbol *func,
                               struct Assembly *assembly,
                               struct CodegenContext *ctx)
{
    if (!node) {
        // Just return a register holding 0 if needed
        int dummy_reg = allocate_register(ctx);
        add_instruction(text, INSTR_MOV, reg_operand(dummy_reg), imm_operand(0));
        return dummy_reg;
    }

    // Integer literal
    if (node->type == NODE_INTEGER_LITERAL) {
        int r = allocate_register(ctx);
        add_instruction(text, INSTR_MOV, reg_operand(r),
                        imm_operand(node->int_literal.value));
        return r;
    }

    // Identifier
    else if (node->type == NODE_IDENTIFIER) {
        struct Symbol *var = lookup_symbol(func->function.locals, node->identifier.name);
        // If not found here, might be in global or parent scope, etc.
        if (var) {
            int r = allocate_register(ctx);
            add_instruction(text, INSTR_MOV, reg_operand(r),
                            mem_operand(REG_RBP, var->variable.offset));
            return r;
        }
        // fallback error
        fprintf(stderr, "generate_expression: unknown identifier '%s'\n", node->identifier.name);
        exit(1);
    }

    // String literal
    else if (node->type == NODE_STRING_LITERAL) {
        // Put string in data section, load it with LEA
        char *label = add_string_literal(assembly, node->string_literal.value);
        int r = allocate_register(ctx);
        add_instruction(text, INSTR_LEA, reg_operand(r), label_operand(label));
        return r;
    }

    // Function call
    else if (node->type == NODE_FUNCTION_CALL) {
        // Save the current usage state.
        int saved_used[REG_COUNT];
        memcpy(saved_used, ctx->used, sizeof(saved_used));

        // Push only the *actually in-use* scratch registers, and mark them as free.
        // We only use caller saved registers as scratch, so we can push all of them.
        int i = 0;
        while (i < REG_COUNT) {
            if (ctx->used[i]) {
                add_instruction(text, INSTR_PUSH, reg_operand(i + 1), reg_operand(i + 1));
                ctx->used[i] = 0;
            }
            i++;
        }

        // Evaluate arguments left to right pushing them into reg_args
        int reg_args[] = {REG_RDI, REG_RSI, REG_RDX, REG_RCX, REG_R8, REG_R9};
        struct ASTNode *arg = node->func_call.arguments;
        int arg_count = 0;
        while (arg && arg_count < 6) {
            int r = generate_expression(text, arg, func, assembly, ctx);
            if (reg_args[arg_count] != r) {
                add_instruction(text, INSTR_MOV, reg_operand(reg_args[arg_count]), reg_operand(r));
            }
            // Free the temporary holding this argument.
            free_register(ctx, r);

            // Mark this argument register as used so it doesn't get reused when
            // evaluating the next argument
            ctx->used[reg_args[arg_count] - 1] = 1;
            arg = arg->next;
            arg_count++;
        }
        // Clear AL for variadic calls
        add_instruction(text, INSTR_MOV, reg_operand(REG_RAX), imm_operand(0));

        // Call the function
        add_instruction(text, INSTR_CALL, label_operand(node->func_call.name),
                        label_operand(node->func_call.name));

        // Pop and restore the usage state for all registers we saved.
        i = REG_COUNT - 1;
        while (i >= 0) {
            if (saved_used[i]) {
                add_instruction(text, INSTR_POP, reg_operand(i + 1), reg_operand(i + 1));
            }
            // Restore usage state exactly as it was before.
            ctx->used[i] = saved_used[i];
            i--;
        }

        // The returned value is in RAX. We want it in a fresh temporary register.
        int result_reg = allocate_register(ctx);
        add_instruction(text, INSTR_MOV, reg_operand(result_reg), reg_operand(REG_RAX));

        return result_reg;
    }

    // Binary operation: left op right
    else if (node->type == NODE_BINARY_OPERATION) {
        // Depth-first: evaluate left
        int left_reg = generate_expression(text, node->binary_op.left, func, assembly, ctx);
        // Evaluate right
        int right_reg = generate_expression(text, node->binary_op.right, func, assembly, ctx);

        // Perform the operation
        if (strcmp(node->binary_op.operator, "+") == 0) {
            add_instruction(text, INSTR_ADD,
                            reg_operand(left_reg), reg_operand(right_reg));
        } else if (strcmp(node->binary_op.operator, "-") == 0) {
            add_instruction(text, INSTR_SUB,
                            reg_operand(left_reg), reg_operand(right_reg));
        } else if (strcmp(node->binary_op.operator, "*") == 0) {
            add_instruction(text, INSTR_MUL,
                            reg_operand(left_reg), reg_operand(right_reg));
        } else if (strcmp(node->binary_op.operator, "/") == 0) {
            // 1. If RDX is used for some *other* expression (not left or right),
            //    then we need to move that occupant out of RDX so we can safely zero RDX.
            //    We'll check if ctx->used[REG_RDX - 1] == 1
            //    but RDX is NOT (left_reg) and NOT (right_reg).
            if (ctx->used[REG_RDX - 1] == 1) {
                if (left_reg != REG_RDX && right_reg != REG_RDX) {
                    // Some other temp is in RDX, so we must move it out.
                    int spare = allocate_register(ctx);
                    add_instruction(text, INSTR_MOV, reg_operand(spare), reg_operand(REG_RDX));
                    // Now we have that occupant in 'spare', so we can free RDX.
                    free_register(ctx, REG_RDX);
                }
            }

            // 2. If the left operand is in RDX, we can move it straight to RAX.
            //    That way, we're not losing its value when we zero RDX.
            if (left_reg == REG_RDX) {
                add_instruction(text, INSTR_MOV, reg_operand(REG_RAX), reg_operand(REG_RDX));
                free_register(ctx, REG_RDX);
                left_reg = REG_RAX;
            }

            // 3. If the right operand is in RDX, we must move it to another temp
            //    so as not to lose it when we zero RDX.
            if (right_reg == REG_RDX) {
                int tmp = allocate_register(ctx);
                add_instruction(text, INSTR_MOV, reg_operand(tmp), reg_operand(REG_RDX));
                free_register(ctx, REG_RDX);
                right_reg = tmp;
            }

            // 4. Move 'left' into RAX if it's not already there.
            //    Then we can free left_reg from the context.
            if (left_reg != REG_RAX) {
                add_instruction(text, INSTR_MOV, reg_operand(REG_RAX), reg_operand(left_reg));
                free_register(ctx, left_reg);
            } else {
                // If left_reg == RAX, we just free it in the context
                free_register(ctx, left_reg);
            }

            // 5. Zero out RDX before idiv (RDX:RAX is the dividend).
            add_instruction(text, INSTR_MOV, reg_operand(REG_RDX), imm_operand(0));

            // 6. Perform IDIV by the right operand → result appears in RAX.
            struct Operand no_op = {0};
            add_instruction(text, INSTR_DIV, reg_operand(right_reg), no_op);

            // 7. We no longer need the right_reg operand.
            free_register(ctx, right_reg);

            // 8. The final result is in RAX. Allocate a fresh scratch reg to hold it.
            int div_res = allocate_register(ctx);
            add_instruction(text, INSTR_MOV, reg_operand(div_res), reg_operand(REG_RAX));
            return div_res;
        } else if (strcmp(node->binary_op.operator, "==") == 0) {
            // Compare left and right values
            add_instruction(text, INSTR_CMP, reg_operand(left_reg), reg_operand(right_reg));
            
            // Set AL to 1 if equal, 0 otherwise
            add_instruction(text, INSTR_SET_EQ, reg_operand(REG_AL), reg_operand(REG_AL));
            
            // Move zero-extended byte to result register
            int result_reg = allocate_register(ctx);
            add_instruction(text, INSTR_MOVZX, reg_operand(result_reg), reg_operand(REG_AL));
            
            free_register(ctx, left_reg);
            free_register(ctx, right_reg);
            return result_reg;
        }

        // If it wasn't division, then left_reg now holds the result.
        free_register(ctx, right_reg);
        // We keep left_reg as final result
        return left_reg;
    }

    // Assignment: target = value
    else if (node->type == NODE_ASSIGNMENT) {
        // Evaluate the right-hand side
        int value_reg = generate_expression(text, node->assignment.value, func, assembly, ctx);

        // target must be an identifier
        if (node->assignment.target->type == NODE_IDENTIFIER) {
            struct Symbol *var = lookup_symbol(func->function.locals,
                                               node->assignment.target->identifier.name);
            if (!var) {
                fprintf(stderr, "Assignment to unknown variable '%s'\n",
                        node->assignment.target->identifier.name);
                exit(1);
            }
            // store it
            add_instruction(text, INSTR_MOV,
                            mem_operand(REG_RBP, var->variable.offset),
                            reg_operand(value_reg));
        }
        free_register(ctx, value_reg);

        // No special "result" register for an assignment, but
        // return a register with the assigned value if you prefer:
        int dummy_reg = allocate_register(ctx);
        add_instruction(text, INSTR_MOV, reg_operand(dummy_reg), imm_operand(0));
        return dummy_reg;
    }

    fprintf(stderr, "generate_expression: unhandled node type %d\n", node->type);
    exit(1);
}

// ------------------- Function that generates code for the entire AST -------------------

// We show only the parts of generate_code function or function body code that remove
// the old special cases and call generate_expression.

// ...
struct Assembly *generate_code(struct ASTNode *ast, struct SemanticContext *context) {
    struct Assembly *assembly = create_assembly();
    add_extern_symbol(assembly, "printf");

    struct Section *text = create_section(".text");
    assembly->sections = text;

    // For each function in the AST
    struct ASTNode *current = ast;
    while (current) {
        if (current->type == NODE_FUNCTION_DECLARATION) {
            struct Symbol *func = lookup_symbol(context->global_scope,
                                                current->function_decl.name);
            if (!func) continue;

            // Add function label
            struct Instruction *label = malloc(sizeof(struct Instruction));
            label->type = INSTR_LABEL;
            label->src.type = OPERAND_LABEL;
            label->src.label = strdup(current->function_decl.name);
            label->next = NULL;

            if (!text->instructions) {
                text->instructions = label;
            } else {
                struct Instruction *last = text->instructions;
                while (last->next) {
                    last = last->next;
                }
                last->next = label;
            }

            // Function prologue
            add_instruction(text, INSTR_PUSH, reg_operand(REG_RBP), reg_operand(REG_RBP));
            add_instruction(text, INSTR_MOV, reg_operand(REG_RBP), reg_operand(REG_RSP));

            // Reserve stack space for all variables
            if (func->function.stack_size > 0) {
                add_instruction(text, INSTR_SUB, reg_operand(REG_RSP),
                              imm_operand(func->function.stack_size));
            }

            // Save parameters to their stack locations
            for (int i = 0; i < current->function_decl.param_count && i < 6; i++) {
                struct Symbol *param = lookup_symbol(func->function.locals,
                                                  current->function_decl.parameters[i].name);
                if (param) {
                    int reg_args[] = {REG_RDI, REG_RSI, REG_RDX, REG_RCX, REG_R8, REG_R9};
                    add_instruction(text, INSTR_MOV,
                                  mem_operand(REG_RBP, param->variable.offset),
                                  reg_operand(reg_args[i]));
                }
            }

            // We'll have a codegen context for each function,
            // re-initialized per statement. Named variables remain on stack after each statement.
            struct CodegenContext ctx_stmt;  // local
            struct ASTNode *body = current->function_decl.body;
            int has_return = 0;

            while (body) {
                // Re-init context each statement
                init_codegen_context(&ctx_stmt);

                if (body->type == NODE_VARIABLE_DECLARATION) {
                    // If there's an initializer, we use generate_expression:
                    if (body->var_decl.value) {
                        int reg = generate_expression(text, body->var_decl.value,
                                                      func, assembly, &ctx_stmt);
                        // store to stack
                        struct Symbol *var = lookup_symbol(func->function.locals,
                                                           body->var_decl.name);
                        add_instruction(text, INSTR_MOV,
                                        mem_operand(REG_RBP, var->variable.offset),
                                        reg_operand(reg));
                        free_register(&ctx_stmt, reg);
                    }
                }
                else if (body->type == NODE_RETURN_STATEMENT) {
                    // Generate the expression into a register
                    int reg = generate_expression(text, body->return_stmt.value,
                                                  func, assembly, &ctx_stmt);
                    // Move that register into RAX
                    add_instruction(text, INSTR_MOV, reg_operand(REG_RAX),
                                    reg_operand(reg));
                    free_register(&ctx_stmt, reg);
                    has_return = 1;

                    // Function epilogue and return
                    add_instruction(text, INSTR_MOV, reg_operand(REG_RSP), reg_operand(REG_RBP));
                    add_instruction(text, INSTR_POP, reg_operand(REG_RBP), reg_operand(REG_RBP));
                    add_instruction(text, INSTR_RET, reg_operand(REG_RAX), reg_operand(REG_RAX));
                }
                else {
                    // For function calls, assignments, binary ops, etc.
                    // we just call generate_expression and discard if needed
                    generate_expression(text, body, func, assembly, &ctx_stmt);
                }

                // Next statement
                body = body->next;
            }

            if (!has_return) {
                add_instruction(text, INSTR_MOV, reg_operand(REG_RSP), reg_operand(REG_RBP));
                add_instruction(text, INSTR_POP, reg_operand(REG_RBP), reg_operand(REG_RBP));
                add_instruction(text, INSTR_RET, reg_operand(REG_RAX), reg_operand(REG_RAX));
            }
        }
        current = current->next;
    }

    return assembly;
}
