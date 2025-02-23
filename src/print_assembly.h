#include "common.h"
#include <stdio.h>

// Convert register number to string
const char *reg_to_str(int reg) {
  if (reg == REG_RAX)
    return "rax";
  if (reg == REG_RBX)
    return "rbx";
  if (reg == REG_RCX)
    return "rcx";
  if (reg == REG_RDX)
    return "rdx";
  if (reg == REG_RSP)
    return "rsp";
  if (reg == REG_RBP)
    return "rbp";
  if (reg == REG_RDI)
    return "rdi";
  if (reg == REG_RSI)
    return "rsi";
  if (reg == REG_R8)
    return "r8";
  if (reg == REG_R9)
    return "r9";
  if (reg == REG_R10)
    return "r10";
  if (reg == REG_R11)
    return "r11";
  if (reg == REG_R12)
    return "r12";
  if (reg == REG_R13)
    return "r13";
  if (reg == REG_R14)
    return "r14";
  if (reg == REG_R15)
    return "r15";
  if (reg == REG_AL)
    return "al";
  return "unknown";
}

// Convert instruction type to string
const char *instr_to_str(int type) {
  if (type == INSTR_MOV)
    return "movq";
  if (type == INSTR_ADD)
    return "addq";
  if (type == INSTR_SUB)
    return "subq";
  if (type == INSTR_PUSH)
    return "pushq";
  if (type == INSTR_POP)
    return "popq";
  if (type == INSTR_CALL)
    return "call";
  if (type == INSTR_RET)
    return "ret";
  if (type == INSTR_LEA)
    return "leaq";
  if (type == INSTR_MUL)
    return "imulq";
  if (type == INSTR_DIV)
    return "idivq";
  if (type == INSTR_LABEL)
    return "label";
  if (type == INSTR_CMP)
    return "cmpq";
  if (type == INSTR_SET_EQ)
    return "sete";
  if (type == INSTR_SET_NE)
    return "setne";
  if (type == INSTR_MOVZX)
    return "movzbq";
  if (type == INSTR_JE)
    return "je";
  if (type == INSTR_JMP)
    return "jmp";
  return "unknown";
}

// Print an operand
void print_operand(FILE *out, struct Operand op) {
  if (op.type == OPERAND_EMPTY) {
    return;
  }
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
    fprintf(out, "%s", op.label);
  } else if (op.type == OPERAND_RIP_LABEL) {
    fprintf(out, "%s(%%rip)", op.label);
  }
}

// Print an instruction
void print_instruction(FILE *out, struct Instruction *instr) {
  fprintf(out, "    %s ", instr_to_str(instr->type));

  // Print first operand if it exists
  print_operand(out, instr->op1);

  // Print second operand if it exists and first operand wasn't empty
  if (instr->op2.type != OPERAND_EMPTY && instr->op1.type != OPERAND_EMPTY) {
    fprintf(out, ", ");
    print_operand(out, instr->op2);
  }

  fprintf(out, "\n");
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

  // Print each section
  struct Section *section = assembly->sections;
  while (section) {
    struct Instruction *instr = section->instructions;
    while (instr) {
      if (instr->type == INSTR_LABEL) {
        fprintf(out, "%s:\n", instr->op1.label);
      } else {
        print_instruction(out, instr);
      }
      instr = instr->next;
    }
    section = section->next;
  }
}
