#pragma once

#include <cstdlib>
#include <string>
#include <iostream>

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
    std::unique_ptr<BaseAST> stmt;

    void Dump() const override {
        std::cout << "BlockAST { ";
        stmt->Dump();
        std::cout << " }";
    }
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
    std::unique_ptr<BaseAST> exp;

    void Dump() const override {}
};

class ExpAST : public BaseAST {
public:
    std::unique_ptr<BaseAST> add_exp;
    
    void Dump() const override {}
};

class PrimaryExpAST : public BaseAST {
public:
    std::string type; // "exp" or "number"
    std::unique_ptr<BaseAST> exp;
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
