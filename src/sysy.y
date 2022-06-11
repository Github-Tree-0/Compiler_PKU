%code requires {
  #include <memory>
  #include <string>
  #include "ast.hpp"
}

%{

#include <iostream>
#include <memory>
#include <string>
#include <vector>
#include "ast.hpp"
#include <cassert>

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
  std::vector<std::unique_ptr<BaseAST> > *vec_val;
}

// lexer 返回的所有 token 种类的声明
// 注意 IDENT 和 INT_CONST 会返回 token 的值, 分别对应 str_val 和 int_val
%token INT RETURN CONST IF ELSE WHILE BREAK CONTINUE VOID
%token <str_val> IDENT
%token <int_val> INT_CONST
%token <str_val> RELOP EQOP ANDOP OROP

// 非终结符的类型定义
%type <ast_val> FuncDef Block Stmt Exp PrimaryExp UnaryExp AddExp MulExp
%type <ast_val> RelExp EqExp LAndExp LOrExp Decl ConstDecl ConstDef ConstInitVal BlockItem
%type <ast_val> ConstExp VarDecl VarDef InitVal OpenStmt ClosedStmt SimpleStmt FuncFParam
%type <ast_val> CompUnitSupp LVal
%type <vec_val> BlockItem_List ConstDef_List VarDef_List FuncFParams FuncRParams
%type <int_val> Number
%type <str_val> UnaryOp Type
%type <vec_val> Index_List ConstInitVal_List InitVal_List ExpIndex_List

%%

CompUnit
  : CompUnitSupp {
    auto comp_unit = unique_ptr<CompUnitAST>((CompUnitAST*)($1));
    ast = move(comp_unit);
  }
  ;

CompUnitSupp
  : FuncDef {
    auto comp_unit_supp = new CompUnitAST();
    comp_unit_supp->func_def_list.push_back(unique_ptr<BaseAST>($1));
    $$ = comp_unit_supp;
  }
  | Decl {
    auto comp_unit_supp = new CompUnitAST();
    comp_unit_supp->global_decl_list.push_back(unique_ptr<BaseAST>($1));
    $$ = comp_unit_supp;
  }
  | CompUnitSupp FuncDef {
    CompUnitAST *comp_unit_supp = (CompUnitAST*)($1);
    comp_unit_supp->func_def_list.push_back(unique_ptr<BaseAST>($2));
    $$ = comp_unit_supp;
  }
  | CompUnitSupp Decl {
    CompUnitAST *comp_unit_supp = (CompUnitAST*)($1);
    comp_unit_supp->global_decl_list.push_back(unique_ptr<BaseAST>($2));
    $$ = comp_unit_supp;
  }
  ;

FuncDef
  : Type IDENT '(' ')' Block {
    auto func_def = new FuncDefAST();
    func_def->func_type = *unique_ptr<std::string>($1);
    func_def->ident = *unique_ptr<string>($2);
    func_def->block = unique_ptr<BaseAST>($5);
    $$ = func_def;
  }
  | Type IDENT '(' FuncFParams ')' Block {
    auto func_def = new FuncDefAST();
    func_def->func_type = *unique_ptr<std::string>($1);
    func_def->ident = *unique_ptr<string>($2);
    vector<unique_ptr<BaseAST> > *v_ptr = ($4);
    for (auto iter = v_ptr->begin(); iter != v_ptr->end(); iter++)
      func_def->func_f_params.push_back(move(*iter));
    func_def->block = unique_ptr<BaseAST>($6);
    $$ = func_def;
  }
  ;


Type
  : INT {
    string *type = new string("int");
    $$ = type;
  }
  | VOID {
    string *type = new string("void");
    $$ = type;
  }
  ;

Block
  : '{' BlockItem_List '}' {
    auto block = new BlockAST();
    vector<unique_ptr<BaseAST> > *v_ptr = ($2);
    for (auto iter = v_ptr->begin(); iter != v_ptr->end(); iter++)
      block->block_item_list.push_back(move(*iter));
    $$ = block;
  }
  ;

Stmt
  : OpenStmt {
    auto stmt = ($1);
    $$ = stmt;
  }
  | ClosedStmt {
    auto stmt = ($1);
    $$ = stmt;
  }
  ;

ClosedStmt
  : SimpleStmt {
    auto stmt = new StmtAST();
    stmt->type = "simple";
    stmt->exp_simple = unique_ptr<BaseAST>($1);
    $$ = stmt;
  }
  | IF '(' Exp ')' ClosedStmt ELSE ClosedStmt {
    auto stmt = new StmtAST();
    stmt->type = "ifelse";
    stmt->exp_simple = unique_ptr<BaseAST>($3);
    stmt->if_stmt = unique_ptr<BaseAST>($5);
    stmt->else_stmt = unique_ptr<BaseAST>($7);
    $$ = stmt;
  }
  | WHILE '(' Exp ')' ClosedStmt {
    auto stmt = new StmtAST();
    stmt->type = "while";
    stmt->exp_simple = unique_ptr<BaseAST>($3);
    stmt->while_stmt = unique_ptr<BaseAST>($5);
    $$ = stmt;
  }
  ;

OpenStmt
  : IF '(' Exp ')' Stmt {
    auto stmt = new StmtAST();
    stmt->type = "if";
    stmt->exp_simple = unique_ptr<BaseAST>($3);
    stmt->if_stmt = unique_ptr<BaseAST>($5);
    $$ = stmt;
  }
  | IF '(' Exp ')' ClosedStmt ELSE OpenStmt {
    auto stmt = new StmtAST();
    stmt->type = "ifelse";
    stmt->exp_simple = unique_ptr<BaseAST>($3);
    stmt->if_stmt = unique_ptr<BaseAST>($5);
    stmt->else_stmt = unique_ptr<BaseAST>($7);
    $$ = stmt;
  }
  | WHILE '(' Exp ')' OpenStmt {
    auto stmt = new StmtAST();
    stmt->type = "while";
    stmt->exp_simple = unique_ptr<BaseAST>($3);
    stmt->while_stmt = unique_ptr<BaseAST>($5);
    $$ = stmt;
  }
  ;

SimpleStmt
  : RETURN Exp ';' {
    auto stmt = new SimpleStmtAST();
    stmt->type = "ret";
    stmt->block_exp = unique_ptr<BaseAST>($2);
    $$ = stmt;
  }
  | RETURN ';' {
    auto stmt = new SimpleStmtAST();
    stmt->type = "ret";
    stmt->block_exp = nullptr;
    $$ = stmt;
  }
  | LVal '=' Exp ';' {
    auto stmt = new SimpleStmtAST();
    stmt->type = "lval";
    stmt->l_val = unique_ptr<BaseAST>($1);
    stmt->block_exp = unique_ptr<BaseAST>($3);
    $$ = stmt;
  }
  | Block {
    auto stmt = new SimpleStmtAST();
    stmt->type = "block";
    stmt->block_exp = unique_ptr<BaseAST>($1);
    $$ = stmt;
  }
  | Exp ';' {
    auto stmt = new SimpleStmtAST();
    stmt->type = "exp";
    stmt->block_exp = unique_ptr<BaseAST>($1);
    $$ = stmt;
  }
  | ';' {
    auto stmt = new SimpleStmtAST();
    stmt->type = "exp";
    stmt->block_exp = nullptr;
    $$ = stmt;
  }
  | BREAK ';' {
    auto stmt = new SimpleStmtAST();
    stmt->type = "break";
    $$ = stmt;
  }
  | CONTINUE ';' {
    auto stmt = new SimpleStmtAST();
    stmt->type = "continue";
    $$ = stmt;
  }
  ;

Exp
  : LOrExp {
    auto exp = new ExpAST();
    exp->lor_exp = unique_ptr<BaseAST>($1);
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
  | LVal {
    auto primary_exp = new PrimaryExpAST();
    primary_exp->type = "lval";
    primary_exp->l_val = unique_ptr<BaseAST>($1);
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
  | IDENT '(' ')' {
    auto unary_exp = new UnaryExpAST();
    unary_exp->type = "call";
    unary_exp->ident = *unique_ptr<string>($1);
    $$ = unary_exp;
  }
  | IDENT '(' FuncRParams ')' {
    auto unary_exp = new UnaryExpAST();
    unary_exp->type = "call";
    unary_exp->ident = *unique_ptr<string>($1);
    vector<unique_ptr<BaseAST> > *v_ptr = ($3);
    for (auto iter = v_ptr->begin(); iter != v_ptr->end(); iter++)
      unary_exp->func_r_params.push_back(move(*iter));
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

MulExp
  : UnaryExp {
    auto mul_exp = new MulExpAST();
    mul_exp->op = "";
    mul_exp->unary_exp = unique_ptr<BaseAST>($1);
    $$ = mul_exp;
  }
  | MulExp '*' UnaryExp {
    auto mul_exp = new MulExpAST();
    mul_exp->mul_exp = unique_ptr<BaseAST>($1);
    mul_exp->op = "*";
    mul_exp->unary_exp = unique_ptr<BaseAST>($3);
    $$ = mul_exp;
  }
  | MulExp '/' UnaryExp {
    auto mul_exp = new MulExpAST();
    mul_exp->mul_exp = unique_ptr<BaseAST>($1);
    mul_exp->op = "/";
    mul_exp->unary_exp = unique_ptr<BaseAST>($3);
    $$ = mul_exp;
  }
  | MulExp '%' UnaryExp {
    auto mul_exp = new MulExpAST();
    mul_exp->mul_exp = unique_ptr<BaseAST>($1);
    mul_exp->op = "%";
    mul_exp->unary_exp = unique_ptr<BaseAST>($3);
    $$ = mul_exp;
  }
  ;

AddExp
  : MulExp {
    auto add_exp = new AddExpAST();
    add_exp->op = "";
    add_exp->mul_exp = unique_ptr<BaseAST>($1);
    $$ = add_exp;
  }
  | AddExp '+' MulExp {
    auto add_exp = new AddExpAST();
    add_exp->op = "+";
    add_exp->add_exp = unique_ptr<BaseAST>($1);
    add_exp->mul_exp = unique_ptr<BaseAST>($3);
    $$ = add_exp;
  }
  | AddExp '-' MulExp {
    auto add_exp = new AddExpAST();
    add_exp->op = "-";
    add_exp->add_exp = unique_ptr<BaseAST>($1);
    add_exp->mul_exp = unique_ptr<BaseAST>($3);
    $$ = add_exp;
  }
  ;

RelExp
  : AddExp {
    auto rel_exp = new RelExpAST();
    rel_exp->op = "";
    rel_exp->add_exp = unique_ptr<BaseAST>($1);
    $$ = rel_exp;
  }
  | RelExp RELOP AddExp {
    auto rel_exp = new RelExpAST();
    rel_exp->rel_exp = unique_ptr<BaseAST>($1);
    rel_exp->op = *unique_ptr<string>($2);
    rel_exp->add_exp = unique_ptr<BaseAST>($3);
    $$ = rel_exp;
  }
  ;

EqExp
  : RelExp {
    auto eq_exp = new EqExpAST();
    eq_exp->op = "";
    eq_exp->rel_exp = unique_ptr<BaseAST>($1);
    $$ = eq_exp;
  }
  | EqExp EQOP RelExp {
    auto eq_exp = new EqExpAST();
    eq_exp->eq_exp = unique_ptr<BaseAST>($1);
    eq_exp->op = *unique_ptr<string>($2);
    eq_exp->rel_exp = unique_ptr<BaseAST>($3);
    $$ = eq_exp;
  }
  ;

LAndExp
  : EqExp {
    auto land_exp = new LAndExpAST();
    land_exp->op = "";
    land_exp->eq_exp = unique_ptr<BaseAST>($1);
    $$ = land_exp;
  }
  | LAndExp ANDOP EqExp {
    auto land_exp = new LAndExpAST();
    land_exp->land_exp = unique_ptr<BaseAST>($1);
    land_exp->op = *unique_ptr<string>($2);
    land_exp->eq_exp = unique_ptr<BaseAST>($3);
    $$ = land_exp;
  }
  ;

LOrExp
  : LAndExp {
    auto lor_exp = new LOrExpAST();
    lor_exp->op = "";
    lor_exp->land_exp = unique_ptr<BaseAST>($1);
    $$ = lor_exp;
  }
  | LOrExp OROP LAndExp {
    auto lor_exp = new LOrExpAST();
    lor_exp->lor_exp = unique_ptr<BaseAST>($1);
    lor_exp->op = *unique_ptr<string>($2);
    lor_exp->land_exp = unique_ptr<BaseAST>($3);
    $$ = lor_exp;
  }
  ;

Decl
  : ConstDecl {
    auto decl = new DeclAST();
    decl->type = "const_decl";
    decl->decl = unique_ptr<BaseAST>($1);
    $$ = decl;
  }
  | VarDecl {
    auto decl = new DeclAST();
    decl->type = "var_decl";
    decl->decl = unique_ptr<BaseAST>($1);
    $$ = decl;
  }
  ;

ConstDecl
  : CONST Type ConstDef_List ';' {
    auto const_decl = new ConstDeclAST();
    const_decl->b_type = *unique_ptr<string>($2);
    assert(const_decl->b_type == "int");
    vector<unique_ptr<BaseAST> > *v_ptr = ($3);
    for (auto iter = v_ptr->begin(); iter != v_ptr->end(); iter++)
      const_decl->const_def_list.push_back(move(*iter));
    $$ = const_decl;
  }
  ;

ConstDef
  : IDENT '=' ConstInitVal {
    auto const_def = new ConstDefAST();
    const_def->ident = *unique_ptr<string>($1);
    const_def->const_init_val = unique_ptr<BaseAST>($3);
    $$ = const_def;
  }
  | IDENT Index_List '=' ConstInitVal {
    auto const_def = new ConstDefAST();
    const_def->ident = *unique_ptr<string>($1);
    vector<unique_ptr<BaseAST> > *v_ptr = ($2);
    for (auto iter = v_ptr->begin(); iter != v_ptr->end(); iter++)
      const_def->const_exps.push_back(move(*iter));
    const_def->const_init_val = unique_ptr<BaseAST>($4);
    $$ = const_def;
  }
  ;

ConstInitVal
  : ConstExp {
    auto const_init_val = new ConstInitValAST();
    const_init_val->const_exp = unique_ptr<BaseAST>($1);
    $$ = const_init_val;
  }
  |
  '{' '}' {
    auto const_init_val = new ConstInitValAST();
    const_init_val->const_exp = nullptr;
    $$ = const_init_val;
  }
  | '{' ConstInitVal_List '}' {
    auto const_init_val = new ConstInitValAST();
    const_init_val->const_exp = nullptr;
    vector<unique_ptr<BaseAST> > *v_ptr = ($2);
    for (auto iter = v_ptr->begin(); iter != v_ptr->end(); iter++)
      const_init_val->const_init_vals.push_back(move(*iter));
    $$ = const_init_val;
  }
  ;

BlockItem
  : Decl {
    auto block_item = new BlockItemAST();
    block_item->type = "decl";
    block_item->content = unique_ptr<BaseAST>($1);
    $$ = block_item;
  }
  | Stmt {
    auto block_item = new BlockItemAST();
    block_item->type = "stmt";
    block_item->content = unique_ptr<BaseAST>($1);
    $$ = block_item;
  }
  ;

LVal
  : IDENT {
    auto l_val = new LValAST();
    l_val->ident = *unique_ptr<string>($1);
    $$ = l_val;
  }
  | IDENT ExpIndex_List {
    auto l_val = new LValAST();
    l_val->ident = *unique_ptr<string>($1);
    vector<unique_ptr<BaseAST> > *v_ptr = ($2);
    for (auto iter = v_ptr->begin(); iter != v_ptr->end(); iter++)
      l_val->exps.push_back(move(*iter));
    $$ = l_val;
  }
  ;
  
ConstExp
  : Exp {
    auto const_exp = new ConstExpAST();
    const_exp->exp = unique_ptr<BaseAST>($1);
    $$ = const_exp;
  }
  ;

VarDecl
  : Type VarDef_List ';' {
    auto var_decl = new VarDeclAST();
    var_decl->b_type = *unique_ptr<string>($1);
    assert(var_decl->b_type == "int");
    vector<unique_ptr<BaseAST> > *v_ptr = ($2);
    for (auto iter = v_ptr->begin(); iter != v_ptr->end(); iter++)
      var_decl->var_def_list.push_back(move(*iter));
    $$ = var_decl;
  }
  ;

VarDef
  : IDENT {
    auto var_def = new VarDefAST();
    var_def->ident = *unique_ptr<string>($1);
    var_def->init_val = nullptr;
    $$ = var_def;
  }
  | IDENT '=' InitVal {
    auto var_def = new VarDefAST();
    var_def->ident = *unique_ptr<string>($1);
    var_def->init_val = unique_ptr<BaseAST>($3);
    $$ = var_def;
  }
  | IDENT Index_List {
    auto var_def = new VarDefAST();
    var_def->ident = *unique_ptr<string>($1);
    var_def->init_val = nullptr;
    vector<unique_ptr<BaseAST> > *v_ptr = ($2);
    for (auto iter = v_ptr->begin(); iter != v_ptr->end(); iter++)
      var_def->const_exps.push_back(move(*iter));
    $$ = var_def;
  }
  | IDENT Index_List '=' InitVal {
    auto var_def = new VarDefAST();
    var_def->ident = *unique_ptr<string>($1);
    var_def->init_val = unique_ptr<BaseAST>($4);
    vector<unique_ptr<BaseAST> > *v_ptr = ($2);
    for (auto iter = v_ptr->begin(); iter != v_ptr->end(); iter++)
      var_def->const_exps.push_back(move(*iter));
    $$ = var_def;
  }
  ;

InitVal
  : Exp {
    auto init_val = new InitValAST();
    init_val->exp = unique_ptr<BaseAST>($1);
    $$ = init_val;
  }
  | '{' '}' {
    auto init_val = new InitValAST();
    init_val->exp = nullptr;
    $$ = init_val;
  }
  | '{' InitVal_List '}' {
    auto init_val = new InitValAST();
    init_val->exp = nullptr;
    vector<unique_ptr<BaseAST> > *v_ptr = ($2);
    for (auto iter = v_ptr->begin(); iter != v_ptr->end(); iter++)
      init_val->init_vals.push_back(move(*iter));
    $$ = init_val;
  }
  ;

BlockItem_List
  : {
    vector<unique_ptr<BaseAST> > *v = new vector<unique_ptr<BaseAST> >;
    $$ = v;
  }
  | BlockItem_List BlockItem {
    vector<unique_ptr<BaseAST> > *v = ($1);
    v->push_back(unique_ptr<BaseAST>($2));
    $$ = v;
  }
  ;

ConstDef_List
  : ConstDef {
    vector<unique_ptr<BaseAST> > *v = new vector<unique_ptr<BaseAST> >;
    v->push_back(unique_ptr<BaseAST>($1));
    $$ = v;
  }
  | ConstDef_List ',' ConstDef {
    vector<unique_ptr<BaseAST> > *v = ($1);
    v->push_back(unique_ptr<BaseAST>($3));
    $$ = v;
  }
  ;

VarDef_List
  : VarDef {
    vector<unique_ptr<BaseAST> > *v = new vector<unique_ptr<BaseAST> >;
    v->push_back(unique_ptr<BaseAST>($1));
    $$ = v;
  }
  | VarDef_List ',' VarDef {
    vector<unique_ptr<BaseAST> > *v = ($1);
    v->push_back(unique_ptr<BaseAST>($3));
    $$ = v;
  }
  ;

FuncFParams
  : FuncFParam {
    vector<unique_ptr<BaseAST> > *v = new vector<unique_ptr<BaseAST> >;
    v->push_back(unique_ptr<BaseAST>($1));
    $$ = v;
  }
  | FuncFParams ',' FuncFParam {
    vector<unique_ptr<BaseAST> > *v = ($1);
    v->push_back(unique_ptr<BaseAST>($3));
    $$ = v;
  }
  ;

FuncFParam
  : Type IDENT {
    auto func_f_param = new FuncFParamAST();
    func_f_param->b_type = *unique_ptr<string>($1);
    assert(func_f_param->b_type == "int");
    func_f_param->ident = *unique_ptr<string>($2);
    $$ = func_f_param;
  }
  ;

FuncRParams
  : Exp {
    vector<unique_ptr<BaseAST> > *v = new vector<unique_ptr<BaseAST> >;
    v->push_back(unique_ptr<BaseAST>($1));
    $$ = v;
  }
  | FuncRParams ',' Exp {
    vector<unique_ptr<BaseAST> > *v = ($1);
    v->push_back(unique_ptr<BaseAST>($3));
    $$ = v;
  }
  ;

Index_List
  : '[' ConstExp ']' {
    vector<unique_ptr<BaseAST> > *v = new vector<unique_ptr<BaseAST> >;
    v->push_back(unique_ptr<BaseAST>($2));
    $$ = v;
  }
  | Index_List '[' ConstExp ']' {
    vector<unique_ptr<BaseAST> > *v = ($1);
    v->push_back(unique_ptr<BaseAST>($3));
    $$ = v;
  }
  ;

ConstInitVal_List
  : ConstInitVal {
    vector<unique_ptr<BaseAST> > *v = new vector<unique_ptr<BaseAST> >;
    v->push_back(unique_ptr<BaseAST>($1));
    $$ = v;
  }
  | ConstInitVal_List ',' ConstInitVal {
    vector<unique_ptr<BaseAST> > *v = ($1);
    v->push_back(unique_ptr<BaseAST>($3));
    $$ = v;
  }
  ;

InitVal_List
  : InitVal {
    vector<unique_ptr<BaseAST> > *v = new vector<unique_ptr<BaseAST> >;
    v->push_back(unique_ptr<BaseAST>($1));
    $$ = v;
  }
  | InitVal_List ',' InitVal {
    vector<unique_ptr<BaseAST> > *v = ($1);
    v->push_back(unique_ptr<BaseAST>($3));
    $$ = v;
  }
  ;

ExpIndex_List
  : '[' Exp ']' {
    vector<unique_ptr<BaseAST> > *v = new vector<unique_ptr<BaseAST> >;
    v->push_back(unique_ptr<BaseAST>($2));
    $$ = v;
  }
  | ExpIndex_List '[' Exp ']' {
    vector<unique_ptr<BaseAST> > *v = ($1);
    v->push_back(unique_ptr<BaseAST>($3));
    $$ = v;
  }
  ;

%%

// 定义错误处理函数, 其中第二个参数是错误信息
// parser 如果发生错误 (例如输入的程序出现了语法错误), 就会调用这个函数
void yyerror(unique_ptr<BaseAST> &ast, const char *s) {
  cerr << "error: " << s << endl;
}