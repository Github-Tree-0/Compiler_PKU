#pragma once

#include <string>
#include <iostream>
#include <map>
#include <algorithm>
#include "koopa.h"

struct Reg_Add {
  int reg;
  int address;
  std::string glob_add;
};

int global_var_cnt = 0;

int stack_top = 0;
int stack_size = 0;
int ra_pos = -1; // -1 means no leaf function
std::string reg_names[16] = { "t0", "t1", "t2", "t3", "t4", "t5", "t6", 
                              "a0", "a1", "a2", "a3", "a4", "a5", "a6", 
                              "a7", "x0" }; // "x0" is not changable
koopa_raw_value_t registers[16]; // value
int reg_stats[16] = {0}; // 0: empty, 1: used but evictable, 2: used and unevictable

koopa_raw_value_t present_value = 0; // 当前正访问的指令
koopa_raw_function_t present_function = 0; // 当前正在访问的函数

std::map<const koopa_raw_value_t, Reg_Add> value_map;

// Declaration of the functions
void Visit(const koopa_raw_program_t &program);
void Visit(const koopa_raw_slice_t &slice);
void Visit(const koopa_raw_function_t &func);
void Visit(const koopa_raw_basic_block_t &bb);
Reg_Add Visit(const koopa_raw_value_t &value);
void Visit(const koopa_raw_return_t &ret);
Reg_Add Visit(const koopa_raw_integer_t &integer);
Reg_Add Visit(const koopa_raw_binary_t &binary);
Reg_Add Visit(const koopa_raw_load_t &load);
void Visit(const koopa_raw_store_t &store);
void Visit(const koopa_raw_branch_t &branch);
void Visit(const koopa_raw_jump_t &jump);
Reg_Add Visit(const koopa_raw_call_t &call);
int Alloc_register(int stat);
Reg_Add Visit(const koopa_raw_global_alloc_t &global);
void save_all_regs();

void save_all_regs() {
  for (int i = 0; i < 15; ++i) {
    if (reg_stats[i] == 1) {
      bool is_global = (value_map[registers[i]].glob_add != "");
      if (is_global) {
        // dirty bits stuff
        assert(false);
      }
      else {
        value_map[registers[i]].reg = -1;
        int add = value_map[registers[i]].address;
        if (add == -1) {
          add = stack_top;
          stack_top += 4;
          value_map[registers[i]].address = add;
          std::cout << "  " << "sw " << reg_names[i] << ", " << std::to_string(add) << "(sp)" << std::endl;
        }
      }
    }
    reg_stats[i] = 0;
  }
}

int Alloc_register(int stat) {
  for (int i = 0; i < 15; ++i) {
    if (reg_stats[i] == 0) {
      registers[i] = present_value;
      reg_stats[i] = stat;
      return i;
    }
  }
  for (int i = 0; i < 15; ++i) {
    if (reg_stats[i] == 1) {
      value_map[registers[i]].reg = -1;
      bool is_global = (value_map[registers[i]].glob_add != "");
      if (is_global) {
        // 后面补充dirty bits之类的操作
        assert(false);
      }
      else {
        int add = value_map[registers[i]].address;
        if (add == -1) {
          add = stack_top;
          stack_top += 4;
          value_map[registers[i]].address = add;
          std::cout << "  " << "sw " << reg_names[i] << ", " << std::to_string(add) << "(sp)" << std::endl;
        }
      }
      registers[i] = present_value;
      reg_stats[i] = stat;
      return i;
    }
  }
  assert(false);
  return -1;
}

// 访问 raw program
void Visit(const koopa_raw_program_t &program) {
  // 访问所有全局变量
  std::cout << "  .data" << std::endl;
  Visit(program.values);
  std::cout << std::endl;
  // 访问所有函数
  Visit(program.funcs);
}

// 访问 raw slice
void Visit(const koopa_raw_slice_t &slice) {
  for (size_t i = 0; i < slice.len; ++i) {
    auto ptr = slice.buffer[i];
    // 根据 slice 的 kind 决定将 ptr 视作何种元素
    switch (slice.kind) {
      case KOOPA_RSIK_FUNCTION:
        // 访问函数
        Visit(reinterpret_cast<koopa_raw_function_t>(ptr));
        break;
      case KOOPA_RSIK_BASIC_BLOCK:
        // 访问基本块
        Visit(reinterpret_cast<koopa_raw_basic_block_t>(ptr));
        break;
      case KOOPA_RSIK_VALUE:
        // 访问指令
        Visit(reinterpret_cast<koopa_raw_value_t>(ptr));
        break;
      default:
        // 我们暂时不会遇到其他内容, 于是不对其做任何处理
        assert(false);
    }
  }
}

// 访问函数
void Visit(const koopa_raw_function_t &func) {
  if (func->bbs.len == 0) // decl
    return;
  
  present_function = func;
  stack_top = 0; stack_size = 0;
  ra_pos = -1; // -1 means no leaf function
  for (int i = 0; i < 15 ; ++i)
    reg_stats[i] = 0;
  present_value = 0; // 当前正访问的指令
  std::map<const koopa_raw_value_t, Reg_Add> old_value_map = value_map;

  std::cout << "  " << ".text" << std::endl;
  std::cout << "  " << ".globl " << (func->name+1) << std::endl;
  std::cout << func->name+1 << ":" << std::endl;
  // Calculate the stack memory space
  int args_num = 0, save_ra = 0;
  for (size_t i = 0; i < func->bbs.len; ++i) {
    koopa_raw_basic_block_t bb_ptr = reinterpret_cast<koopa_raw_basic_block_t>(func->bbs.buffer[i]);
    for (size_t j = 0; j < bb_ptr->insts.len; ++j) {
      koopa_raw_value_t inst_ptr = reinterpret_cast<koopa_raw_value_t>(bb_ptr->insts.buffer[j]);
      if (inst_ptr->ty->tag != KOOPA_RTT_UNIT || inst_ptr->kind.tag == KOOPA_RVT_ALLOC)
        stack_size += 4;
      if (inst_ptr->kind.tag == KOOPA_RVT_CALL) {
        save_ra = 1;
        args_num = std::max((int)inst_ptr->kind.data.call.args.len, args_num);
      }
    }
  }
  int overflow_byte = 4 * std::max(args_num - 8, 0);
  stack_size += overflow_byte; stack_top += overflow_byte;
  if (save_ra)
    stack_size += 4;

  size_t size_mod = stack_size % 16;
  if (size_mod)
    stack_size += 16 - size_mod;
  if (stack_size) {
    if (stack_size <= 2048)
      std::cout << "  " << "addi sp, sp, " << std::to_string(-stack_size) << std::endl;
    else {
      std::cout << "  " << "li t0, " << std::to_string(-stack_size) << std::endl;
      std::cout << "  " << "addi sp, sp, t0" << std::endl;
    }
  }

  if (save_ra) {
    ra_pos = stack_size-4;
    std::cout << "  " << "sw ra, " << std::to_string(ra_pos) << "(sp)" << std::endl;
  }

  // Deal with the params
  for (size_t i = 0; i < func->params.len; ++i) {
    koopa_raw_value_t param = reinterpret_cast<koopa_raw_value_t>(func->params.buffer[i]);
    if (i < 8) { // stored in ax
      int reg_index = i + 7;
      registers[reg_index] = param;
      reg_stats[reg_index] = 1;
      struct Reg_Add var = {reg_index, -1, ""};
      value_map[param] = var;
    }
    else { // stored in the stack
      int address = stack_size + (i - 8) * 4;
      struct Reg_Add var = {-1, address, ""};
      value_map[param] = var;
    }
  }
  
  Visit(func->bbs);
  std::cout << std::endl;

  value_map = old_value_map;
}

// 访问基本块
void Visit(const koopa_raw_basic_block_t &bb) {
  // 访问所有指令
  std::string bb_name(bb->name+1);
  if (bb_name != "entry")
    std::cout << present_function->name+1 << "_" << bb_name << ":" << std::endl;
  Visit(bb->insts);
}

// 访问指令
Reg_Add Visit(const koopa_raw_value_t &value) {
  // 返回时值一定在寄存器里
  koopa_raw_value_t old_value = present_value;
  present_value = value;
  struct Reg_Add result_var = {-1, -1, ""};

  if (value_map.count(value)) {
    if (value_map[value].reg == -1) {
      if (value_map[value].glob_add == "") {
        int reg = Alloc_register(1);
        value_map[value].reg = reg;
        std::cout << "  " << "lw " << reg_names[reg] << ", " << std::to_string(value_map[value].address) << "(sp)" << std::endl;
        result_var = value_map[value];
      }
      else { // global
        assert(false);
        // dirty bits needed
      }
    }
    else
      result_var = value_map[value];
    present_value = old_value;
    return result_var;
  }

  const auto &kind = value->kind;
  switch (kind.tag) {
    case KOOPA_RVT_RETURN:
      // 访问 return 指令
      Visit(kind.data.ret);
      break;
    case KOOPA_RVT_INTEGER:
      // 访问 integer 指令
      result_var = Visit(kind.data.integer); // not gonna save this
      break;
    case KOOPA_RVT_BINARY:
      result_var = Visit(kind.data.binary);
      value_map[value] = result_var;
      break;
    case KOOPA_RVT_ALLOC:
      result_var.address = stack_top;
      stack_top += 4;
      value_map[value] = result_var;
      break;
    case KOOPA_RVT_LOAD:
      result_var = Visit(kind.data.load);
      value_map[value] = result_var;
      break;
    case KOOPA_RVT_STORE:
      Visit(kind.data.store);
      break;
    case KOOPA_RVT_BRANCH:
      Visit(kind.data.branch);
      break;
    case KOOPA_RVT_JUMP:
      Visit(kind.data.jump);
      break;
    case KOOPA_RVT_CALL:
      result_var = Visit(kind.data.call);
      if (result_var.reg != -1)
        value_map[value] = result_var;
      break;
    case KOOPA_RVT_FUNC_ARG_REF:
      assert(false); // All args are defined upon calling the function
      break;
    case KOOPA_RVT_GLOBAL_ALLOC:
      result_var = Visit(kind.data.global_alloc);
      value_map[value] = result_var;
      break;
    default:
      // 其他类型暂时遇不到
      assert(false);
  }

  present_value = old_value; // 防止递归时改掉值

  return result_var;
}

void Visit(const koopa_raw_return_t &ret) {
  // 这里没有考虑直接li到a0的做法
  koopa_raw_value_t ret_value = ret.value;
  if (ret_value != NULL) {
    struct Reg_Add result_var = Visit(ret_value);
    if (result_var.reg != 7)
      std::cout << "  " << "mv a0, " << reg_names[result_var.reg] << std::endl;
  }
  if (ra_pos != -1)
    std::cout << "  " << "lw ra, " << std::to_string(ra_pos) << "(sp)" << std::endl;
  if (stack_size) {
    if (stack_size <= 2048)
      std::cout << "  " << "addi sp, sp, " << std::to_string(stack_size) << std::endl;
    else {
      std::cout << "  " << "li t0, " << std::to_string(stack_size) << std::endl;
      std::cout << "  " << "addi sp, sp, t0" << std::endl;
    }
  }
  std::cout << "  " << "ret" << std::endl;
}

Reg_Add Visit(const koopa_raw_integer_t &integer) {
  // integer stored in registers, thus returns register number
  int32_t int_val = integer.value;
  struct Reg_Add result_var = {-1, -1, ""};
  if (int_val == 0) {
    result_var.reg = 15;
    return result_var;
  }
  result_var.reg = Alloc_register(0);
  std::cout << "  " << "li " << reg_names[result_var.reg] << ", " << std::to_string(int_val) << std::endl;

  return result_var;
}

Reg_Add Visit(const koopa_raw_binary_t &binary) {
  // returns memory address
  koopa_raw_binary_op_t op = binary.op;
  struct Reg_Add left_val = Visit(binary.lhs);
  int left_register = left_val.reg;
  int old_stat = reg_stats[left_register];
  reg_stats[left_register] = 2;
  struct Reg_Add right_val = Visit(binary.rhs);
  int right_register = right_val.reg;
  reg_stats[left_register] = old_stat;
  struct Reg_Add result_var = {Alloc_register(1), -1, ""};

  std::string left_reg_name = reg_names[left_register], right_reg_name = reg_names[right_register];
  std::string result_reg_name = reg_names[result_var.reg];

  switch (op) {
    case KOOPA_RBO_EQ:
      std::cout << "  " << "xor " << result_reg_name << ", " << left_reg_name << ", " << right_reg_name << std::endl;
      std::cout << "  " << "seqz " << result_reg_name << ", " << result_reg_name << std::endl;
      break;
    case KOOPA_RBO_SUB:
      std::cout << "  " << "sub " << result_reg_name << ", " << left_reg_name << ", " << right_reg_name << std::endl;
      break;
    case KOOPA_RBO_ADD:
      std::cout << "  " << "add " << result_reg_name << ", " << left_reg_name << ", " << right_reg_name << std::endl;
      break;
    case KOOPA_RBO_MUL:
      std::cout << "  " << "mul " << result_reg_name << ", " << left_reg_name << ", " << right_reg_name << std::endl;
      break;
    case KOOPA_RBO_DIV:
      std::cout << "  " << "div " << result_reg_name << ", " << left_reg_name << ", " << right_reg_name << std::endl;
      break;
    case KOOPA_RBO_MOD:
      std::cout << "  " << "rem " << result_reg_name << ", " << left_reg_name << ", " << right_reg_name << std::endl;
      break;
    case KOOPA_RBO_NOT_EQ:
      std::cout << "  " << "xor " << result_reg_name << ", " << left_reg_name << ", " << right_reg_name << std::endl;
      std::cout << "  " << "snez " << result_reg_name << ", " << result_reg_name << std::endl;
      break;
    case KOOPA_RBO_LT:
      std::cout << "  " << "slt " << result_reg_name << ", " << left_reg_name << ", " << right_reg_name << std::endl;
      break;
    case KOOPA_RBO_LE:
      std::cout << "  " << "sgt " << result_reg_name << ", " << left_reg_name << ", " << right_reg_name << std::endl;
      std::cout << "  " << "xori " << result_reg_name << ", " << result_reg_name << ", 1" << std::endl; 
      break;
    case KOOPA_RBO_AND:
      std::cout << "  " << "and " << result_reg_name << ", " << left_reg_name << ", " << right_reg_name << std::endl;
      break;
    case KOOPA_RBO_OR:
      std::cout << "  " << "or " << result_reg_name << ", " << left_reg_name << ", " << right_reg_name << std::endl;
      break;
    case KOOPA_RBO_GT:
      std::cout << "  " << "sgt " << result_reg_name << ", " << left_reg_name << ", " << right_reg_name << std::endl;
      break;
    case KOOPA_RBO_GE:
      std::cout << "  " << "slt " << result_reg_name << ", " << left_reg_name << ", " << right_reg_name << std::endl;
      std::cout << "  " << "xori " << result_reg_name << ", " << result_reg_name << ", 1" << std::endl; 
      break;
    default:
      assert(false);
  }

  return result_var;
}

Reg_Add Visit(const koopa_raw_load_t &load) {
  koopa_raw_value_t src = load.src;
  bool is_local = (value_map[src].address != -1), is_global = (value_map[src].glob_add != "");
  assert(value_map.count(src) && (is_local || is_global));
  int reg = Alloc_register(1);
  struct Reg_Add result_var = {reg, -1, ""};
  if (is_local) {
    int address = value_map[src].address;
    std::cout << "  " << "lw " << reg_names[reg] << ", " << std::to_string(address) << "(sp)" << std::endl;
  }
  else { // global
    std::string label = value_map[src].glob_add;
    std::cout << "  " << "la s1, " << label << std::endl; // s1 是干这个的专用寄存器
    std::cout << "  " << "lw " << reg_names[reg] << ", 0(s1)" << std::endl;
  }
  return result_var;
}

void Visit(const koopa_raw_store_t &store) {
  struct Reg_Add value = Visit(store.value);
  koopa_raw_value_t dest = store.dest;
  assert(value_map.count(dest));
  bool is_global = (value_map[dest].glob_add != "");
  int reg = value.reg;
  if (is_global) {
    std::string label = value_map[dest].glob_add;
    std::cout << "  " << "la s1, " << label << std::endl;
    std::cout << "  " << "sw " << reg_names[reg] << ", 0(s1)" << std::endl;
  }
  else { // local
    if (value_map[dest].address == -1) {
      value_map[dest].address = stack_top;
      stack_top += 4;
    }
    int address = value_map[dest].address;
    std::cout << "  " << "sw " << reg_names[reg] << ", " << std::to_string(address) << "(sp)" << std::endl;
  }
}

void Visit(const koopa_raw_branch_t &branch) {
  std::string func_name(present_function->name+1), true_name(branch.true_bb->name+1), false_name(branch.false_bb->name+1);
  std::string true_label = func_name + "_" + true_name;
  std::string false_label = func_name + "_" + false_name;
  int cond_reg = Visit(branch.cond).reg;
  save_all_regs();
  std::cout << "  " << "bnez " << reg_names[cond_reg] << ", " << true_label << std::endl;
  std::cout << "  " << "j " << false_label << std::endl;
}

void Visit(const koopa_raw_jump_t &jump) {
  save_all_regs(); // start a new block, clear all registers
  std::string func_name(present_function->name+1), target_name(jump.target->name+1);
  std::string target = func_name + "_" + target_name;
  std::cout << "  " << "j " << target << std::endl;
}

Reg_Add Visit(const koopa_raw_call_t &call) {
  struct Reg_Add result_var = {-1, -1, ""};
  for (int i = 0; i < call.args.len; ++i) {
    koopa_raw_value_t arg_ptr = reinterpret_cast<koopa_raw_value_t>(call.args.buffer[i]);
    int arg = Visit(arg_ptr).reg;
    if (i < 8) {
      int reg_index = i + 7;
      if (arg != reg_index) { // mv result to ai
        if (reg_stats[reg_index] == 1) { // clear ai
          value_map[registers[reg_index]].reg = -1;
          bool is_global = (value_map[registers[reg_index]].glob_add != "");
          if (is_global) {
            // dirty bits stuff
            assert(false);
          }
          else {
            int add = value_map[registers[reg_index]].address;
            if (add == -1) {
              add = stack_top;
              stack_top += 4;
              value_map[registers[reg_index]].address = add;
              std::cout << "  " << "sw " << reg_names[reg_index] << ", " << std::to_string(add) << "(sp)" << std::endl;
            }
          }
        }
        std::cout << "  " << "mv " << reg_names[reg_index] << ", " << reg_names[arg] << std::endl;
        reg_stats[reg_index] = 2;
      }
    }
    else {
      int address = (i - 8) * 4;
      std::cout << "  " << "sw " << reg_names[arg] << ", " << std::to_string(address) << "(sp)" << std::endl;
    }
  }

  save_all_regs();

  std::cout << "  " << "call " << call.callee->name+1 << std::endl;

  if (present_value->ty->tag != KOOPA_RTT_UNIT) { // return value stored in a0
    registers[7] = present_value;
    reg_stats[7] = 1;
    result_var.reg = 7;
  }
  
  return result_var;
}

Reg_Add Visit(const koopa_raw_global_alloc_t &global) {
  std::string label = "global_var_" + std::to_string(global_var_cnt++);
  std::cout << "  " << ".globl " << label << std::endl;
  std::cout << label << ":" << std::endl;

  const auto &kind = global.init->kind;
  if (kind.tag == KOOPA_RVT_INTEGER) {
    int32_t int_val = kind.data.integer.value;
    std::cout << "  " << ".word " << std::to_string(int_val) << std::endl;
  }
  else if (kind.tag == KOOPA_RVT_ZERO_INIT)
    std::cout << "  .zero 4" << std::endl;
  else
    assert(false);

  struct Reg_Add result_var = {-1, -1, label};

  return result_var;
}