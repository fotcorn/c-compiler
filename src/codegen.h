#include "common.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// All used temp regs are caller saved, which avoid needing to handle
// push/pop at the function level for callee saved registers.
// We also don't use RAX for simplicity.
static const int TEMP_REGS[] = {
    REG_R10, // Never used for params, good first choice
    REG_R11, // Never used for params
    REG_R9,  // Only used for 6th param (rare)
    REG_R8,  // Only used for 5th
    REG_RCX, // 4th param
    REG_RDX, // 3rd param
    REG_RSI, // 2nd param
    REG_RDI  // 1st param (used most often for params)
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
                                     assembly->extern_count * sizeof(char *));
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
void add_instruction(struct Section *section, int type, struct Operand op1,
                     struct Operand op2) {
  struct Instruction *instr = malloc(sizeof(struct Instruction));
  instr->type = type;
  instr->op1 = op1;
  instr->op2 = op2;
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

struct Operand rip_label_operand(const char *label) {
  struct Operand op = {.type = OPERAND_RIP_LABEL};
  op.label = strdup(label);
  return op;
}

struct Operand empty_operand() {
  struct Operand op = {.type = OPERAND_EMPTY};
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
// We'll define a few more registers we can use as scratch registers for
// expressions. A small struct to keep track of which registers are currently in
// use. For simplicity, we store a small fixed array of possible scratch
// registers. "used[i] = 1" if that register is already allocated, 0 if free.
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

// --------------------- Unified Expression Generation
// ------------------------------- Instead of special per-node code in each
// place, we define one function that recursively generates code for any
// expression and returns the reg holding the final result.

static int generate_expression(struct Section *text, struct ASTNode *node,
                               struct Symbol *func, struct Assembly *assembly,
                               struct CodegenContext *ctx);

// Implementation of generate_expression:
static int generate_expression(struct Section *text, struct ASTNode *node,
                               struct Symbol *func, struct Assembly *assembly,
                               struct CodegenContext *ctx) {
  assert(node && "generate_expression: node cannot be null");

  // Integer literal
  if (node->type == NODE_INTEGER_LITERAL) {
    int r = allocate_register(ctx);
    add_instruction(text, INSTR_MOV, imm_operand(node->int_literal.value),
                    reg_operand(r));
    return r;
  }

  // Identifier
  else if (node->type == NODE_IDENTIFIER) {
    // Use the stack offset stored in the AST node during semantic analysis
    int r = allocate_register(ctx);
    add_instruction(text, INSTR_MOV,
                    mem_operand(REG_RBP, node->identifier.stack_offset),
                    reg_operand(r));
    return r;
  }

  // String literal
  else if (node->type == NODE_STRING_LITERAL) {
    // Put string in data section, load it with LEA
    char *label = add_string_literal(assembly, node->string_literal.value);
    int r = allocate_register(ctx);
    add_instruction(text, INSTR_LEA, rip_label_operand(label), reg_operand(r));
    return r;
  }

  // Function call
  else if (node->type == NODE_FUNCTION_CALL) {
    // Save the current usage state.
    int saved_used[REG_COUNT];
    memcpy(saved_used, ctx->used, sizeof(saved_used));

    // Push only the *actually in-use* scratch registers, and mark them as free.
    // We only use caller saved registers as scratch, so we can push all of
    // them.
    int i = 0;
    while (i < REG_COUNT) {
      if (ctx->used[i]) {
        add_instruction(text, INSTR_PUSH, reg_operand(i + 1), empty_operand());
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
        add_instruction(text, INSTR_MOV, reg_operand(r),
                        reg_operand(reg_args[arg_count]));
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
    add_instruction(text, INSTR_MOV, imm_operand(0), reg_operand(REG_RAX));

    // Call the function
    add_instruction(text, INSTR_CALL, label_operand(node->func_call.name),
                    empty_operand());

    // Pop and restore the usage state for all registers we saved.
    i = REG_COUNT - 1;
    while (i >= 0) {
      if (saved_used[i]) {
        add_instruction(text, INSTR_POP, reg_operand(i + 1), empty_operand());
      }
      // Restore usage state exactly as it was before.
      ctx->used[i] = saved_used[i];
      i--;
    }

    // The returned value is in RAX. We want it in a fresh temporary register.
    int result_reg = allocate_register(ctx);
    add_instruction(text, INSTR_MOV, reg_operand(REG_RAX),
                    reg_operand(result_reg));

    return result_reg;
  }

  // Binary operation: left op right
  else if (node->type == NODE_BINARY_OPERATION) {
    // Depth-first: evaluate left
    int left_reg =
        generate_expression(text, node->binary_op.left, func, assembly, ctx);
    // Evaluate right
    int right_reg =
        generate_expression(text, node->binary_op.right, func, assembly, ctx);

    // Perform the operation
    if (strcmp(node->binary_op.operator, "+") == 0) {
      add_instruction(text, INSTR_ADD, reg_operand(right_reg),
                      reg_operand(left_reg));
    } else if (strcmp(node->binary_op.operator, "-") == 0) {
      add_instruction(text, INSTR_SUB, reg_operand(right_reg),
                      reg_operand(left_reg));
    } else if (strcmp(node->binary_op.operator, "*") == 0) {
      add_instruction(text, INSTR_MUL, reg_operand(right_reg),
                      reg_operand(left_reg));
    } else if (strcmp(node->binary_op.operator, "/") == 0) {
      // 1. If RDX is used for some *other* expression (not left or right),
      //    then we need to move that occupant out of RDX so we can safely zero
      //    RDX. We'll check if ctx->used[REG_RDX - 1] == 1 but RDX is NOT
      //    (left_reg) and NOT (right_reg).
      if (ctx->used[REG_RDX - 1] == 1) {
        if (left_reg != REG_RDX && right_reg != REG_RDX) {
          // Some other temp is in RDX, so we must move it out.
          int spare = allocate_register(ctx);
          add_instruction(text, INSTR_MOV, reg_operand(REG_RDX),
                          reg_operand(spare));
          // Now we have that occupant in 'spare', so we can free RDX.
          free_register(ctx, REG_RDX);
        }
      }

      // 2. If the left operand is in RDX, we can move it straight to RAX.
      //    That way, we're not losing its value when we zero RDX.
      if (left_reg == REG_RDX) {
        add_instruction(text, INSTR_MOV, reg_operand(REG_RDX),
                        reg_operand(REG_RAX));
        free_register(ctx, REG_RDX);
        left_reg = REG_RAX;
      }

      // 3. If the right operand is in RDX, we must move it to another temp
      //    so as not to lose it when we zero RDX.
      if (right_reg == REG_RDX) {
        int tmp = allocate_register(ctx);
        add_instruction(text, INSTR_MOV, reg_operand(REG_RDX),
                        reg_operand(tmp));
        free_register(ctx, REG_RDX);
        right_reg = tmp;
      }

      // 4. Move 'left' into RAX if it's not already there.
      //    Then we can free left_reg from the context.
      if (left_reg != REG_RAX) {
        add_instruction(text, INSTR_MOV, reg_operand(left_reg),
                        reg_operand(REG_RAX));
        free_register(ctx, left_reg);
      } else {
        // If left_reg == RAX, we just free it in the context
        free_register(ctx, left_reg);
      }

      // 5. Zero out RDX before idiv (RDX:RAX is the dividend).
      add_instruction(text, INSTR_MOV, imm_operand(0), reg_operand(REG_RDX));

      // 6. Perform IDIV by the right operand → result appears in RAX.
      add_instruction(text, INSTR_DIV, reg_operand(right_reg), empty_operand());

      // 7. We no longer need the right_reg operand.
      free_register(ctx, right_reg);

      // 8. The final result is in RAX. Allocate a fresh scratch reg to hold it.
      int div_res = allocate_register(ctx);
      add_instruction(text, INSTR_MOV, reg_operand(REG_RAX),
                      reg_operand(div_res));
      return div_res;
    } else if (strcmp(node->binary_op.operator, "==") == 0) {
      // Compare left and right values
      add_instruction(text, INSTR_CMP, reg_operand(right_reg),
                      reg_operand(left_reg));

      // Set AL to 1 if equal, 0 otherwise
      add_instruction(text, INSTR_SET_EQ, reg_operand(REG_AL), empty_operand());

      // Move zero-extended byte to result register
      int result_reg = allocate_register(ctx);
      add_instruction(text, INSTR_MOVZX, reg_operand(REG_AL),
                      reg_operand(result_reg));

      free_register(ctx, left_reg);
      free_register(ctx, right_reg);
      return result_reg;
    } else if (strcmp(node->binary_op.operator, "!=") == 0) {
      // Compare left and right values
      add_instruction(text, INSTR_CMP, reg_operand(right_reg),
                      reg_operand(left_reg));

      // Set AL to 1 if not equal, 0 otherwise
      add_instruction(text, INSTR_SET_NE, reg_operand(REG_AL), empty_operand());

      // Move zero-extended byte to result register
      int result_reg = allocate_register(ctx);
      add_instruction(text, INSTR_MOVZX, reg_operand(REG_AL),
                      reg_operand(result_reg));

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
    int value_reg =
        generate_expression(text, node->assignment.value, func, assembly, ctx);

    // target must be an identifier
    if (node->assignment.target->type == NODE_IDENTIFIER) {
      // Use the stack offset stored in the identifier node
      add_instruction(
          text, INSTR_MOV, reg_operand(value_reg),
          mem_operand(REG_RBP,
                      node->assignment.target->identifier.stack_offset));
    } else {
      fprintf(stderr, "Assignment to non-identifier is not supported\n");
      exit(1);
    }
    free_register(ctx, value_reg);

    return -1;
  }

  fprintf(stderr, "generate_expression: unhandled node type %d\n", node->type);
  exit(1);
}

// Forward declaration of generate_block
static int generate_block(struct Section *text, struct ASTNode *block,
                          struct Symbol *func, struct Assembly *assembly);

// Helper function to output code for an assignment
static void generate_assignment(struct Section *text, struct ASTNode *target,
                                struct ASTNode *value, struct Symbol *func,
                                struct Assembly *assembly,
                                struct CodegenContext *ctx) {
  // Evaluate the right-hand side
  int value_reg = generate_expression(text, value, func, assembly, ctx);

  // Handle different kinds of targets
  if (target->type == NODE_IDENTIFIER) {
    // Use the stack offset stored directly in the identifier node
    add_instruction(text, INSTR_MOV, reg_operand(value_reg),
                    mem_operand(REG_RBP, target->identifier.stack_offset));
  } else {
    fprintf(stderr, "Assignment to non-identifier is not supported\n");
    exit(1);
  }

  // Free the register used for the value
  free_register(ctx, value_reg);
}

// Returns 1 if the block contains a return statement that terminates the block.
static int generate_block(struct Section *text, struct ASTNode *block,
                          struct Symbol *func, struct Assembly *assembly) {
  int has_return = 0;
  while (block) {
    struct CodegenContext ctx_stmt;
    init_codegen_context(&ctx_stmt);
    switch (block->type) {
    case NODE_VARIABLE_DECLARATION:
      if (block->var_decl.value) {
        int reg = generate_expression(text, block->var_decl.value, func,
                                      assembly, &ctx_stmt);
        // Use stored stack offset from var_decl directly
        add_instruction(text, INSTR_MOV, reg_operand(reg),
                        mem_operand(REG_RBP, block->var_decl.stack_offset));
        free_register(&ctx_stmt, reg);
      }
      break;

    case NODE_ASSIGNMENT:
      // Use the helper function for assignments
      generate_assignment(text, block->assignment.target,
                          block->assignment.value, func, assembly, &ctx_stmt);
      break;

    case NODE_RETURN_STATEMENT: {
      int reg = generate_expression(text, block->return_stmt.value, func,
                                    assembly, &ctx_stmt);
      add_instruction(text, INSTR_MOV, reg_operand(reg), reg_operand(REG_RAX));
      free_register(&ctx_stmt, reg);
      // Function epilogue and return
      add_instruction(text, INSTR_MOV, reg_operand(REG_RBP),
                      reg_operand(REG_RSP));
      add_instruction(text, INSTR_POP, reg_operand(REG_RBP), empty_operand());
      add_instruction(text, INSTR_RET, empty_operand(), empty_operand());
      has_return = 1;
      // A return terminates further code generation for this block.
      return has_return;
    }

    case NODE_IF_STATEMENT: {
      static int if_counter = 0;
      int cond_reg = generate_expression(text, block->if_stmt.condition, func,
                                         assembly, &ctx_stmt);
      add_instruction(text, INSTR_CMP, imm_operand(0), reg_operand(cond_reg));
      free_register(&ctx_stmt, cond_reg);

      char else_label[32];
      char end_label[32];
      snprintf(else_label, sizeof(else_label), ".Lelse%d", if_counter);
      snprintf(end_label, sizeof(end_label), ".Lif_end%d", if_counter);
      if_counter++;

      // Jump to else branch if condition is false.
      add_instruction(text, INSTR_JE, label_operand(else_label),
                      empty_operand());

      // Generate the "if" (then) block.
      int then_return =
          generate_block(text, block->if_stmt.body, func, assembly);
      // Jump to end after then block.
      add_instruction(text, INSTR_JMP, label_operand(end_label),
                      empty_operand());

      // Append the else label.
      struct Instruction *else_instr = malloc(sizeof(struct Instruction));
      else_instr->type = INSTR_LABEL;
      else_instr->op1.type = OPERAND_LABEL;
      else_instr->op1.label = strdup(else_label);
      else_instr->next = NULL;
      if (text->instructions) {
        struct Instruction *last = text->instructions;
        while (last->next)
          last = last->next;
        last->next = else_instr;
      } else {
        text->instructions = else_instr;
      }

      // Generate the "else" block.
      int else_return =
          generate_block(text, block->if_stmt.else_body, func, assembly);

      // Append the end label.
      struct Instruction *end_instr = malloc(sizeof(struct Instruction));
      end_instr->type = INSTR_LABEL;
      end_instr->op1.type = OPERAND_LABEL;
      end_instr->op1.label = strdup(end_label);
      end_instr->next = NULL;
      if (text->instructions) {
        struct Instruction *last = text->instructions;
        while (last->next)
          last = last->next;
        last->next = end_instr;
      } else {
        text->instructions = end_instr;
      }

      // If both branches guarantee a return, then mark this block as returning.
      if (then_return && else_return) {
        has_return = 1;
      }
      break;
    }

    case NODE_WHILE_STATEMENT: {
      static int while_counter = 0;
      char start_label[32];
      char end_label[32];
      snprintf(start_label, sizeof(start_label), ".Lwhile_start%d",
               while_counter);
      snprintf(end_label, sizeof(end_label), ".Lwhile_end%d", while_counter);
      while_counter++;

      /* Place start label */
      struct Instruction *start_instr = malloc(sizeof(struct Instruction));
      start_instr->type = INSTR_LABEL;
      start_instr->op1.type = OPERAND_LABEL;
      start_instr->op1.label = strdup(start_label);
      start_instr->next = NULL;
      if (text->instructions) {
        struct Instruction *last = text->instructions;
        while (last->next)
          last = last->next;
        last->next = start_instr;
      } else {
        text->instructions = start_instr;
      }

      /* Evaluate condition */
      struct CodegenContext ctx_cond;
      init_codegen_context(&ctx_cond);
      int cond_reg = generate_expression(text, block->while_stmt.condition,
                                         func, assembly, &ctx_cond);
      add_instruction(text, INSTR_CMP, imm_operand(0), reg_operand(cond_reg));
      free_register(&ctx_cond, cond_reg);

      /* Jump to end if condition is false */
      add_instruction(text, INSTR_JE, label_operand(end_label),
                      empty_operand());

      /* Generate while loop body */
      generate_block(text, block->while_stmt.body, func, assembly);

      /* Jump back to start label */
      add_instruction(text, INSTR_JMP, label_operand(start_label),
                      empty_operand());

      /* Place end label */
      struct Instruction *end_instr = malloc(sizeof(struct Instruction));
      end_instr->type = INSTR_LABEL;
      end_instr->op1.type = OPERAND_LABEL;
      end_instr->op1.label = strdup(end_label);
      end_instr->next = NULL;
      if (text->instructions) {
        struct Instruction *last = text->instructions;
        while (last->next)
          last = last->next;
        last->next = end_instr;
      } else {
        text->instructions = end_instr;
      }
      break;
    }

    default:
      // For expressions (assignments, function calls, binary ops, etc.)
      generate_expression(text, block, func, assembly, &ctx_stmt);
    }
    block = block->next;
  }
  return has_return;
}

// ------------------- Function that generates code for the entire AST
// -------------------

// We show only the parts of generate_code function or function body code that
// remove the old special cases and call generate_expression.

// ...
struct Assembly *generate_code(struct ASTNode *ast,
                               struct SemanticContext *context) {
  struct Assembly *assembly = create_assembly();
  add_extern_symbol(assembly, "printf");

  struct Section *text = create_section(".text");
  assembly->sections = text;

  // For each function in the AST
  struct ASTNode *current = ast;
  while (current) {
    if (current->type == NODE_FUNCTION_DECLARATION) {
      struct Symbol *func =
          lookup_symbol(context->global_scope, current->function_decl.name);
      if (!func)
        continue;

      // Add function label
      struct Instruction *label = malloc(sizeof(struct Instruction));
      label->type = INSTR_LABEL;
      label->op1.type = OPERAND_LABEL;
      label->op1.label = strdup(current->function_decl.name);
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
      add_instruction(text, INSTR_PUSH, reg_operand(REG_RBP), empty_operand());
      add_instruction(text, INSTR_MOV, reg_operand(REG_RSP),
                      reg_operand(REG_RBP));

      // Reserve stack space for all variables
      if (func->function.stack_size > 0) {
        add_instruction(text, INSTR_SUB, imm_operand(func->function.stack_size),
                        reg_operand(REG_RSP));
      }

      // Save parameters to their stack locations
      for (int i = 0; i < current->function_decl.param_count && i < 6; i++) {
        struct Symbol *param = lookup_symbol(
            func->function.locals, current->function_decl.parameters[i].name);
        if (param) {
          int reg_args[] = {REG_RDI, REG_RSI, REG_RDX, REG_RCX, REG_R8, REG_R9};
          add_instruction(text, INSTR_MOV, reg_operand(reg_args[i]),
                          mem_operand(REG_RBP, param->variable.offset));
        }
      }

      // Generate code for the function body using generic block code
      // generation. The block generator returns a flag indicating if a return
      // was encountered.
      int has_return =
          generate_block(text, current->function_decl.body, func, assembly);
      if (!has_return) {
        add_instruction(text, INSTR_MOV, reg_operand(REG_RSP),
                        reg_operand(REG_RBP));
        add_instruction(text, INSTR_POP, reg_operand(REG_RBP), empty_operand());
        add_instruction(text, INSTR_RET, empty_operand(), empty_operand());
      }
    }
    current = current->next;
  }

  return assembly;
}
