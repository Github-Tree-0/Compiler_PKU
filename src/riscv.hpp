#pragma once

#include <string>
#include <iostream>
#include <map>
#include "koopa.h"

struct Reg_Add {
  int reg;
  int address;
};

int stack_top = 0;
int stack_size = 0;
std::string reg_names[16] = { "t0", "t1", "t2", "t3", "t4", "t5", "t6", 
                              "a0", "a1", "a2", "a3", "a4", "a5", "a6", 
                              "a7", "x0" }; // "x0" is not changable
koopa_raw_value_t registers[16]; // value
int reg_stats[16] = {0}; // 0: empty, 1: used but evictable, 2: used and unevictable, 3: used evictable

koopa_raw_value_t present_value = 0; // 当前正访问的指令

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
int Alloc_register(int stat);

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
      int add = value_map[registers[i]].address;
      if (add == -1) {
        add = stack_top;
        stack_top += 4;
        value_map[registers[i]].address = add;
        std::cout << "  " << "sw " << reg_names[i] << ", " << std::to_string(add) << "(sp)" << std::endl;
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
  std::cout << "  " << ".text" << std::endl;
  // 访问所有全局变量
  Visit(program.values);
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
  std::cout << "  " << ".globl " << (func->name+1) << std::endl;
  std::cout << func->name+1 << ":" << std::endl;
  // Calculate the stack memory space
  for (size_t i = 0; i < func->bbs.len; ++i) {
    koopa_raw_basic_block_t bb_ptr = reinterpret_cast<koopa_raw_basic_block_t>(func->bbs.buffer[i]);
    for (size_t j = 0; j < bb_ptr->insts.len; ++j) {
      koopa_raw_value_t inst_ptr = reinterpret_cast<koopa_raw_value_t>(bb_ptr->insts.buffer[j]);
      if (inst_ptr->ty->tag != KOOPA_RTT_UNIT || inst_ptr->kind.tag == KOOPA_RVT_ALLOC)
        stack_size += 4;
    }
  }
  size_t size_mod = stack_size % 16;
  if (size_mod)
    stack_size += 16 - size_mod;
  if (stack_size <= 2048)
    std::cout << "  " << "addi sp, sp, " << std::to_string(-stack_size) << std::endl;
  else {
    std::cout << "  " << "li t0, " << std::to_string(-stack_size) << std::endl;
    std::cout << "  " << "addi sp, sp, t0" << std::endl;
  }
  
  Visit(func->bbs);
}

// 访问基本块
void Visit(const koopa_raw_basic_block_t &bb) {
  // 访问所有指令
  Visit(bb->insts);
}

// 访问指令
Reg_Add Visit(const koopa_raw_value_t &value) {
  // 返回时值一定在寄存器里
  koopa_raw_value_t old_value = present_value;
  present_value = value;
  if (value_map.count(value)) {
    if (value_map[value].reg == -1) {
      int reg = Alloc_register(1);
      value_map[value].reg = reg;
      std::cout << "  " << "lw " << reg_names[reg] << ", " << std::to_string(value_map[value].address) << "(sp)" << std::endl;
    }
    present_value = old_value;
    return value_map[value];
  }

  const auto &kind = value->kind;
  struct Reg_Add result_var = {-1, -1};
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
  struct Reg_Add result_var = Visit(ret_value);
  std::cout << "  " << "mv a0, " << reg_names[result_var.reg] << std::endl;
  if (stack_size <= 2048)
    std::cout << "  " << "addi sp, sp, " << std::to_string(stack_size) << std::endl;
  else {
    std::cout << "  " << "li t0, " << std::to_string(stack_size) << std::endl;
    std::cout << "  " << "addi sp, sp, t0" << std::endl;
  }
  std::cout << "  " << "ret" << std::endl;
}

Reg_Add Visit(const koopa_raw_integer_t &integer) {
  // integer stored in registers, thus returns register number
  int32_t int_val = integer.value;
  struct Reg_Add result_var = {-1, -1};
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
  struct Reg_Add result_var = {Alloc_register(1), -1};

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
  assert(value_map.count(src) && (value_map[src].address != -1));
  int reg = Alloc_register(1), address = value_map[src].address;
  struct Reg_Add result_var = {reg, address};
  std::cout << "  " << "lw " << reg_names[reg] << ", " << std::to_string(address) << "(sp)" << std::endl;

  return result_var;
}

void Visit(const koopa_raw_store_t &store) {
  struct Reg_Add value = Visit(store.value);
  koopa_raw_value_t dest = store.dest;
  assert(value_map.count(dest) && (value_map[dest].address != -1));
  int reg = value.reg, address = value_map[dest].address;
  std::cout << "  " << "sw " << reg_names[reg] << ", " << std::to_string(address) << "(sp)" << std::endl;
}