#pragma once

#include <cstdlib>
#include <string>
#include <iostream>
#include <vector>

class BaseAST {
public:
    virtual ~BaseAST() = default;
    virtual void Dump() const = 0;
};

class CompUnitAST : public BaseAST {
public:
    std::unique_ptr<BaseAST> func_def;

    void Dump() const override {
        std::cout << "CompUnitAST { ";
        func_def->Dump();
        std::cout << " }";
    }
};

class FuncDefAST : public BaseAST {
public:
    std::unique_ptr<BaseAST> func_type;
    std::string ident;
    std::unique_ptr<BaseAST> block;

    void Dump() const override {
        std::cout << "FuncDefAST { ";
        func_type->Dump();
        std::cout << ", " << ident << ", ";
        block->Dump();
        std::cout << " }";
    }
};

class BlockAST : public BaseAST {
public:
    std::vector<std::unique_ptr<BaseAST> > block_item_list;

    void Dump() const override {}
};

class FuncTypeAST : public BaseAST {
public:
    std::string type;

    void Dump() const override {
        std::cout << "FuncTypeAST { ";
        std::cout << type;
        std::cout << " }";
    }
};

class StmtAST : public BaseAST {
public:
    std::string type; // "if", "ifelse" or "simple"
    std::unique_ptr<BaseAST> exp_simple;
    std::unique_ptr<BaseAST> if_stmt;
    std::unique_ptr<BaseAST> else_stmt;

    void Dump() const override {}
};

class SimpleStmtAST : public BaseAST {
public:
    std::string type; // "lval", "exp", "block" or "ret"
    std::string l_val;
    std::unique_ptr<BaseAST> block_exp;

    void Dump() const override {}
};

class ExpAST : public BaseAST {
public:
    std::unique_ptr<BaseAST> lor_exp;
    
    void Dump() const override {}
};

class PrimaryExpAST : public BaseAST {
public:
    std::string type; // "exp", "number" or "lval"
    std::unique_ptr<BaseAST> exp;
    std::string l_val;
    int number;

    void Dump() const override {}
};

class UnaryExpAST : public BaseAST {
public:
    std::string type; // "primary" or "unary"
    std::unique_ptr<BaseAST> exp;
    std::string op;

    void Dump() const override {}
};

class MulExpAST : public BaseAST {
public:
    std::string op; // "*", "/", "%" or ""
    std::unique_ptr<BaseAST> unary_exp;
    std::unique_ptr<BaseAST> mul_exp;

    void Dump() const override {}
};

class AddExpAST : public BaseAST {
public:
    std::string op; // "+", "-" or ""
    std::unique_ptr<BaseAST> add_exp;
    std::unique_ptr<BaseAST> mul_exp;

    void Dump() const override {}
};

class RelExpAST : public BaseAST {
public:
    std::string op; // "<", ">", "<=", ">=" or ""
    std::unique_ptr<BaseAST> add_exp;
    std::unique_ptr<BaseAST> rel_exp;

    void Dump() const override {}
};

class EqExpAST : public BaseAST {
public:
    std::string op; // "==", "!=" or ""
    std::unique_ptr<BaseAST> eq_exp;
    std::unique_ptr<BaseAST> rel_exp;

    void Dump() const override {}
};

class LAndExpAST : public BaseAST {
public:
    std::string op; // "&&" or ""
    std::unique_ptr<BaseAST> eq_exp;
    std::unique_ptr<BaseAST> land_exp;

    void Dump() const override {}
};

class LOrExpAST : public BaseAST {
public:
    std::string op; // "||" or ""
    std::unique_ptr<BaseAST> land_exp;
    std::unique_ptr<BaseAST> lor_exp;

    void Dump() const override {}
};

class DeclAST : public BaseAST {
public:
    std::string type; // "const_decl" or "var_decl"
    std::unique_ptr<BaseAST> decl;

    void Dump() const override {}
};

class ConstDeclAST : public BaseAST {
public:
    std::string b_type;
    std::vector<std::unique_ptr<BaseAST> > const_def_list;

    void Dump() const override {}
};

class ConstDefAST : public BaseAST {
public:
    std::string ident;
    std::unique_ptr<BaseAST> const_init_val;

    void Dump() const override {}
};

class ConstInitValAST : public BaseAST {
public:
    std::unique_ptr<BaseAST> const_exp;

    void Dump() const override {}
};

class BlockItemAST : public BaseAST {
public:
    std::string type; // "decl" or "stmt"
    std::unique_ptr<BaseAST> content;

    void Dump() const override {}
};

class ConstExpAST : public BaseAST {
public:
    std::unique_ptr<BaseAST> exp;

    void Dump() const override {}
};

class VarDeclAST : public BaseAST {
public:
    std::string b_type;
    std::vector<std::unique_ptr<BaseAST> > var_def_list;

    void Dump() const override {}
};

class VarDefAST : public BaseAST {
public:
    std::string ident;
    bool has_init_val;
    std::unique_ptr<BaseAST> init_val;

    void Dump() const override {}
};

class InitValAST : public BaseAST {
public:
    std::unique_ptr<BaseAST> exp;

    void Dump() const override {}
};