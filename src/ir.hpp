#pragma once

#include <string>
#include <cstdlib>
#include <iostream>
#include <vector>
#include "ast.hpp"

int var_cnt = 0;

void Visit_AST(const CompUnitAST *comp_unit);
void Visit_AST(const FuncDefAST *func_def);
void Visit_AST(const BlockAST *block);
void Visit_AST(const StmtAST *stmt);
std::string Visit_AST(const ExpAST *exp);
std::string Visit_AST(const UnaryExpAST *unary_exp);
std::string Visit_AST(const PrimaryExpAST *primary_exp);
std::string Visit_AST(const MulExpAST *mul_exp);
std::string Visit_AST(const AddExpAST *add_exp);

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
    std::string result_var = Visit_AST((ExpAST*)(stmt->exp.get()));
    std::cout << "  " << "ret " << result_var << std::endl;
}

std::string Visit_AST(const ExpAST *exp) {
    std::string result_var = Visit_AST((AddExpAST*)(exp->add_exp.get()));
    return result_var;
}

std::string Visit_AST(const UnaryExpAST *unary_exp) {
    if (unary_exp->type == "primary") {
        std::string result_var = Visit_AST((PrimaryExpAST*)(unary_exp->exp.get()));
        return result_var;
    }
    else if (unary_exp->type == "unary") {
        std::string result_var = Visit_AST((UnaryExpAST*)(unary_exp->exp.get()));
        std::string next_var = "%" + std::to_string(var_cnt);

        switch (unary_exp->op[0])
        {
        case '+':
            return result_var;
            break;
        case '-':
            std::cout << "  " << next_var << " = sub 0, " << result_var << std::endl;
            break;
        case '!':
            std::cout << "  " << next_var << " = eq " << result_var << ", 0" << std::endl;
            break;
        default:
            assert(false);
        }
        var_cnt++;
        return next_var;
    }
    else {
        assert(false);
    }
    return "";
}

std::string Visit_AST(const PrimaryExpAST *primary_exp) {
    std::string result_var = "";
    if (primary_exp->type == "exp")
        result_var = Visit_AST((ExpAST*)(primary_exp->exp.get()));
    else if (primary_exp->type == "number")
        result_var = std::to_string(primary_exp->number);
    else
        assert(false);

    return result_var;
}

std::string Visit_AST(const MulExpAST *mul_exp) {
    std::string result_var = "";
    if (mul_exp->op == "")
        result_var = Visit_AST((UnaryExpAST*)(mul_exp->unary_exp.get()));
    else {
        std::string left_result = Visit_AST((MulExpAST*)(mul_exp->mul_exp.get()));
        std::string right_result = Visit_AST((UnaryExpAST*)(mul_exp->unary_exp.get()));
        result_var = "%" + std::to_string(var_cnt++);
        switch (mul_exp->op[0])
        {
        case '*':
            std::cout << "  " << result_var << " = mul " << left_result << ", " << right_result << std::endl;
            break;
        case '/':
            std::cout << "  " << result_var << " = div " << left_result << ", " << right_result << std::endl;
            break;
        case '%':
            std::cout << "  " << result_var << " = mod " << left_result << ", " << right_result << std::endl;
            break;
        default:
            assert(false);
            break;
        }
    }

    return result_var;
}

std::string Visit_AST(const AddExpAST *add_exp) {
    std::string result_var = "";
    if (add_exp->op == "")
        result_var = Visit_AST((MulExpAST*)(add_exp->mul_exp.get()));
    else {
        std::string left_result = Visit_AST((AddExpAST*)(add_exp->add_exp.get()));
        std::string right_result = Visit_AST((MulExpAST*)(add_exp->mul_exp.get()));
        result_var = "%" + std::to_string(var_cnt++);
        switch (add_exp->op[0])
        {
        case '+':
            std::cout << "  " << result_var << " = add " << left_result << ", " << right_result << std::endl;
            break;
        case '-':
            std::cout << "  " << result_var << " = sub " << left_result << ", " << right_result << std::endl;
            break;
        default:
            assert(false);
            break;
        }
    }

    return result_var;
}
