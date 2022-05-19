#pragma once

#include <string>
#include <iostream>
#include <map>
#include "koopa.h"

int register_cnt = 0;
int integer_reg_cnt = 0;
std::map<const koopa_raw_value_t, std::string> value_map;

// Declaration of the functions
void Visit(const koopa_raw_program_t &program);
void Visit(const koopa_raw_slice_t &slice);
void Visit(const koopa_raw_function_t &func);
void Visit(const koopa_raw_basic_block_t &bb);
std::string Visit(const koopa_raw_value_t &value);
std::string Visit(const koopa_raw_return_t &ret);
std::string Visit(const koopa_raw_integer_t &integer);
std::string Visit(const koopa_raw_binary_t &binary);

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
  Visit(func->bbs);
}

// 访问基本块
void Visit(const koopa_raw_basic_block_t &bb) {
  // 访问所有指令
  Visit(bb->insts);
}

// 访问指令
std::string Visit(const koopa_raw_value_t &value) {
  // 根据指令类型判断后续需要如何访问
  if (value_map[value] != "")
    return value_map[value];

  const auto &kind = value->kind;
  std::string result_var;
  switch (kind.tag) {
    case KOOPA_RVT_RETURN:
      // 访问 return 指令
      result_var = Visit(kind.data.ret);
      break;
    case KOOPA_RVT_INTEGER:
      // 访问 integer 指令
      result_var = Visit(kind.data.integer);
      break;
    case KOOPA_RVT_BINARY:
      result_var = Visit(kind.data.binary);
      break;
    default:
      // 其他类型暂时遇不到
      assert(false);
  }
  value_map[value] = result_var;

  return result_var;
}

std::string Visit(const koopa_raw_return_t &ret) {
  // 这里没有考虑直接li到a0的做法
  koopa_raw_value_t ret_value = ret.value;
  std::string result_var =  Visit(ret_value);
  std::cout << "  " << "mv a0, " << result_var << std::endl;
  std::cout << "  " << "ret" << std::endl;
  
  return result_var;
}

std::string Visit(const koopa_raw_integer_t &integer) {
  int32_t int_val = integer.value;
  if (int_val == 0)
    return "x0";
  std::string next_var = "t" + std::to_string(integer_reg_cnt++);
  std::cout << "  " << "li " << next_var << ", " << std::to_string(int_val) << std::endl;

  return next_var;
}

std::string Visit(const koopa_raw_binary_t &binary) {
  koopa_raw_binary_op_t op = binary.op;
  integer_reg_cnt = register_cnt;
  std::string left_val = Visit(binary.lhs);
  std::string right_val = Visit(binary.rhs);
  integer_reg_cnt = register_cnt;
  std::string result_var = "t" + std::to_string(register_cnt++);
  switch (op) {
    case KOOPA_RBO_EQ:
      std::cout << "  " << "xor " << result_var << ", " << left_val << ", " << right_val << std::endl;
      std::cout << "  " << "seqz " << result_var << ", " << result_var << std::endl;
      break;
    case KOOPA_RBO_SUB:
      std::cout << "  " << "sub " << result_var << ", " << left_val << ", " << right_val << std::endl;
      break;
    case KOOPA_RBO_ADD:
      std::cout << "  " << "add " << result_var << ", " << left_val << ", " << right_val << std::endl;
      break;
    case KOOPA_RBO_MUL:
      std::cout << "  " << "mul " << result_var << ", " << left_val << ", " << right_val << std::endl;
      break;
    case KOOPA_RBO_DIV:
      std::cout << "  " << "div " << result_var << ", " << left_val << ", " << right_val << std::endl;
      break;
    case KOOPA_RBO_MOD:
      std::cout << "  " << "rem " << result_var << ", " << left_val << ", " << right_val << std::endl;
      break;
    default:
      assert(false);
  }

  return result_var;
}