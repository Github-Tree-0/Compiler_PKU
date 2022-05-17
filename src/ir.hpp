#pragma once

#include <string>
#include <cstdlib>
#include <iostream>
#include <vector>
#include "ast.hpp"

class BaseIR {
public:
    virtual ~BaseIR() = default;
    virtual void Dump() const = 0;
};

class ValueIR : public BaseIR {
public:
    std::string name;
    std::string type;
    int val; // Only support INT values for now
};

class InstructionIR : public BaseIR {
public:
    int type; // 0 for ret
    int args[3];

    void Dump() const override {
        assert(type==0); // Only support instruction "ret"
        std::cout << "ret " << std::to_string(args[0]) << std::endl;
    }
};

class BasicBlockIR : public BaseIR {
public:
    std::string name;
    std::vector<std::unique_ptr<InstructionIR> > insts;

    void Dump() const override {
        std::cout << "%" << name << ":" << std::endl;
        int inst_num = insts.size();
        for (int i = 0; i < inst_num; ++i){
            std::cout << "  ";
            insts[i]->Dump();
        }
    }

    void From_AST(BlockAST *block) {
        // Only 'ret' instruction at present
        auto ret_inst = new InstructionIR();
        ret_inst->type = 0;
        ret_inst->args[0] = ((StmtAST*)(block->stmt.get()))->number;
        insts.push_back((std::unique_ptr<InstructionIR>)ret_inst);
    }
};

class FunctionIR : public BaseIR {
public:
    std::string name;
    std::string type;
    int ret_value; // Only support INT right now
    std::vector<std::unique_ptr<BasicBlockIR> > blocks;

    void Dump() const override {
        std::cout << "fun @" << name << "(): ";
        assert(type=="int");
        std::cout << "i32 {" << std::endl;
        int block_num = blocks.size();
        for (int i = 0; i < block_num; ++i)
            blocks[i]->Dump();
        std::cout << "}" << std::endl;
    }

    void From_AST(FuncDefAST *func_def) {
        name = func_def->ident;
        type = ((FuncTypeAST*)(func_def->func_type.get()))->type;
        auto entry_block = new BasicBlockIR();
        entry_block->name = "entry";
        entry_block->From_AST((BlockAST*)(func_def->block.get()));
        blocks.push_back((std::unique_ptr<BasicBlockIR>)entry_block);
    }
};

class ProgramIR : public BaseIR {
public:
    std::string name;
    std::vector<std::unique_ptr<FunctionIR> > funcs;
    std::vector<std::unique_ptr<FunctionIR> > glob_values;

    void Dump() const override {
        // Without printing glob_values
        int func_num = funcs.size();
        for (int i = 0; i < func_num; ++i)
            funcs[i]->Dump();
    }

    void From_AST(CompUnitAST *comp_unit) {
        auto main_func = new FunctionIR();
        main_func->From_AST((FuncDefAST*)(comp_unit->func_def.get()));
        funcs.push_back((std::unique_ptr<FunctionIR>)main_func);
    }
};