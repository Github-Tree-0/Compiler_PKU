#pragma once

#include <string>
#include <cstdlib>
#include <iostream>
#include <vector>
#include "ast.hpp"

void Visit_AST(const CompUnitAST *comp_unit);
void Visit_AST(const FuncDefAST *func_def);
void Visit_AST(const BlockAST *block);
void Visit_AST(const StmtAST *stmt);

void Visit_AST(const CompUnitAST *comp_unit) {
    Visit_AST((FuncDefAST*)(comp_unit->func_def.get()));
}

void Visit_AST(const FuncDefAST *func_def) {
    std::cout << "fun @" << func_def->ident << "(): ";
    std::string type = ((FuncTypeAST*)(func_def->func_type.get()))->type;
    assert(type=="int");
    std::cout << "i32 {" << std::endl;
    std::cout << "%" << "entry" << ":" << std::endl; // Blocks will have their names in the future
    Visit_AST((BlockAST*)(func_def->block.get()));
    std::cout << "}" << std::endl;
}

void Visit_AST(const BlockAST *block) {
    Visit_AST((StmtAST*)(block->stmt.get()));
}

void Visit_AST(const StmtAST *stmt) {
    // Only support ret instruction at present
    std::cout << "  " << "ret " << std::to_string(stmt->number) << std::endl;
}
