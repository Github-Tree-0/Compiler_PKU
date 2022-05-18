#include <cassert>
#include <cstdio>
#include <iostream>
#include <memory>
#include <string>
#include <stdio.h>
#include <sstream>
#include "ast.hpp"
#include "ir.hpp"
#include "riscv.hpp"
#include "koopa.h"

using namespace std;

extern FILE *yyin;
extern int yyparse(unique_ptr<BaseAST> &ast);

int main(int argc, const char *argv[]) {
  // 解析命令行参数. 测试脚本/评测平台要求你的编译器能接收如下参数:
  // compiler 模式 输入文件 -o 输出文件
  assert(argc == 5);
  string mode = string(argv[1]);
  auto input = argv[2];
  auto output = argv[4];

  // 打开输入文件, 并且指定 lexer 在解析的时候读取这个文件
  yyin = fopen(input, "r");
  //assert(yyin);

  // 调用 parser 函数, parser 函数会进一步调用 lexer 解析输入文件的
  unique_ptr<BaseAST> ast;
  auto ret = yyparse(ast);
  assert(!ret);
  if (mode == "-koopa"){
    freopen(output, "w", stdout); // Redirect the output
    Visit_AST((CompUnitAST*)(ast.get()));
  }
  else if (mode == "-riscv"){
    stringstream ir_ss;
    streambuf *buffer = cout.rdbuf();
    cout.rdbuf(ir_ss.rdbuf());
    Visit_AST((CompUnitAST*)(ast.get()));
    string ir_str = ir_ss.str();
    const char *ir = ir_str.data();
    cout.rdbuf(buffer);

    koopa_program_t program;
    koopa_error_code_t ret = koopa_parse_from_string(ir, &program);
    assert(ret == KOOPA_EC_SUCCESS);  // 确保解析时没有出错
    koopa_raw_program_builder_t builder = koopa_new_raw_program_builder();
    koopa_raw_program_t raw = koopa_build_raw_program(builder, program);
    koopa_delete_program(program);
    
    freopen(output, "w", stdout);
    Visit(raw);

    koopa_delete_raw_program_builder(builder);
  }

  return 0;
}