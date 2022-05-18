%code requires {
  #include <memory>
  #include <string>
  #include "ast.hpp"
}

%{

#include <iostream>
#include <memory>
#include <string>
#include "ast.hpp"

// 声明 lexer 函数和错误处理函数
int yylex();
void yyerror(std::unique_ptr<BaseAST> &ast, const char *s);

using namespace std;

%}

%parse-param { std::unique_ptr<BaseAST> &ast }

%union {
  std::string *str_val;
  int int_val;
  BaseAST *ast_val;
}

// lexer 返回的所有 token 种类的声明
// 注意 IDENT 和 INT_CONST 会返回 token 的值, 分别对应 str_val 和 int_val
%token INT RETURN
%token <str_val> IDENT
%token <int_val> INT_CONST

// 非终结符的类型定义
%type <ast_val> FuncDef FuncType Block Stmt Exp PrimaryExp UnaryExp
%type <int_val> Number
%type <str_val> UnaryOp

%%

CompUnit
  : FuncDef {
    auto comp_unit = make_unique<CompUnitAST>();
    comp_unit->func_def = unique_ptr<BaseAST>($1);
    ast = move(comp_unit);
  }
  ;

FuncDef
  : FuncType IDENT '(' ')' Block {
    auto func_def = new FuncDefAST();
    func_def->func_type = unique_ptr<BaseAST>($1);
    func_def->ident = *unique_ptr<string>($2);
    func_def->block = unique_ptr<BaseAST>($5);
    $$ = func_def;
  }
  ;

FuncType
  : INT {
    auto func_type = new FuncTypeAST();
    func_type->type = "int";
    $$ = func_type;
  }
  ;

Block
  : '{' Stmt '}' {
    auto block = new BlockAST();
    block->stmt = unique_ptr<BaseAST>($2);
    $$ = block;
  }
  ;

Stmt
  : RETURN Exp ';' {
    auto stmt = new StmtAST();
    stmt->exp = unique_ptr<BaseAST>($2);
    $$ = stmt;
  }
  ;

Exp
  : UnaryExp {
    auto exp = new ExpAST();
    exp->unary_exp = unique_ptr<BaseAST>($1);
    $$ = exp;
  }
  ;

PrimaryExp
  : '(' Exp ')' {
    auto primary_exp = new PrimaryExpAST();
    primary_exp->type = "exp";
    primary_exp->exp = unique_ptr<BaseAST>($2);
    $$ = primary_exp;
  }
  | Number {
    auto primary_exp = new PrimaryExpAST();
    primary_exp->type = "number";
    primary_exp->number = ($1);
    $$ = primary_exp;
  }
  ;

Number
  : INT_CONST {
    $$ = ($1);
  }
  ;

UnaryExp
  : PrimaryExp {
    auto unary_exp = new UnaryExpAST();
    unary_exp->type = "primary";
    unary_exp->exp = unique_ptr<BaseAST>($1);
    $$ = unary_exp;
  }
  | UnaryOp UnaryExp {
    auto unary_exp = new UnaryExpAST();
    unary_exp->type = "unary";
    unary_exp->op = *unique_ptr<string>($1);
    unary_exp->exp = unique_ptr<BaseAST>($2);
    $$ = unary_exp;
  }
  ;

UnaryOp
  : '+' {
    string *op = new string("+");
    $$ = op;
  }
  | '-' {
    string *op = new string("-");
    $$ = op;
  }
  | '!' {
    string *op = new string("!");
    $$ = op;
  }
  ;

%%

// 定义错误处理函数, 其中第二个参数是错误信息
// parser 如果发生错误 (例如输入的程序出现了语法错误), 就会调用这个函数
void yyerror(unique_ptr<BaseAST> &ast, const char *s) {
  cerr << "error: " << s << endl;
}