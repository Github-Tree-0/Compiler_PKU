#pragma once

#include <string>
#include <cstdlib>
#include <iostream>
#include <vector>
#include <map>
#include <variant>
#include "ast.hpp"
#include <sstream>

int var_cnt = 0;
std::vector<std::map<std::string, std::variant<int, std::string> > > symbol_tables;
std::map<std::string, int> var_names;
int if_else_cnt = 0;
int other_cnt = 0;

void Visit_AST(const CompUnitAST *comp_unit);
void Visit_AST(const FuncDefAST *func_def);
void Visit_AST(const BlockAST *block);
void Visit_AST(const StmtAST *stmt);
void Visit_AST(const SimpleStmtAST *stmt);
void Visit_AST(const DeclAST *decl);
void Visit_AST(const BlockItemAST *block_item);
void Visit_AST(const ConstDeclAST *const_decl);
void Visit_AST(const ConstDefAST *const_def);
void Visit_AST(const VarDeclAST *var_decl);
void Visit_AST(const VarDefAST *var_def);
std::string Visit_AST(const ExpAST *exp);
std::string Visit_AST(const UnaryExpAST *unary_exp);
std::string Visit_AST(const PrimaryExpAST *primary_exp);
std::string Visit_AST(const MulExpAST *mul_exp);
std::string Visit_AST(const AddExpAST *add_exp);
std::string Visit_AST(const RelExpAST *rel_exp);
std::string Visit_AST(const EqExpAST *eq_exp);
std::string Visit_AST(const LAndExpAST *land_exp);
std::string Visit_AST(const LOrExpAST *lor_exp);
std::string Visit_AST(const InitValAST *init_val);
int Visit_AST(const ConstInitValAST *const_init_val);
int Cal_AST(const ConstExpAST *const_exp);
int Cal_AST(const ExpAST *exp);
int Cal_AST(const UnaryExpAST *unary_exp);
int Cal_AST(const PrimaryExpAST *primary_exp);
int Cal_AST(const MulExpAST *mul_exp);
int Cal_AST(const AddExpAST *add_exp);
int Cal_AST(const RelExpAST *rel_exp);
int Cal_AST(const EqExpAST *eq_exp);
int Cal_AST(const LAndExpAST *land_exp);
int Cal_AST(const LOrExpAST *lor_exp);
std::variant<int, std::string> look_up_symbol_tables(std::string l_val);

std::variant<int, std::string> look_up_symbol_tables(std::string l_val) {
    int size = symbol_tables.size();
    for (int i = size-1; i >= 0; --i) {
        if (symbol_tables[i].count(l_val))
            return symbol_tables[i][l_val];
    }

    assert(false);
    return -1;
}

void Visit_AST(const CompUnitAST *comp_unit) {
    Visit_AST((FuncDefAST*)(comp_unit->func_def.get()));
}

void Visit_AST(const FuncDefAST *func_def) {
    // redir
    std::stringstream ir_ss;
    std::streambuf *buffer = std::cout.rdbuf();
    std::cout.rdbuf(ir_ss.rdbuf());

    std::cout << "fun @" << func_def->ident << "(): ";
    std::string type = ((FuncTypeAST*)(func_def->func_type.get()))->type;
    assert(type=="int");
    std::cout << "i32 {" << std::endl;
    std::cout << "%" << "entry" << ":" << std::endl; // Blocks will have their names in the future
    Visit_AST((BlockAST*)(func_def->block.get()));

    std::string ir_str = ir_ss.str(), last_line = "";
    int pt = ir_str.length() - 2;
    while(ir_str[pt] != '\n')
        last_line = ir_str[pt--] + last_line;
    if (last_line.substr(0, 6) == "%other") // deal with empty ret block
        ir_str = ir_str.substr(0, pt+1);
    std::cout.rdbuf(buffer);

    std::cout << ir_str;
    std::cout << "}" << std::endl;
}

void Visit_AST(const BlockAST *block) {
    std::map<std::string, std::variant<int, std::string> > symbol_table;
    symbol_tables.push_back(symbol_table);
    int size = block->block_item_list.size();
    for (int i = 0; i < size; ++i)
        Visit_AST((BlockItemAST*)(block->block_item_list[i].get()));
    symbol_tables.pop_back();
}

void Visit_AST(const SimpleStmtAST *stmt) {
    if (stmt->type == "ret") {
        if (stmt->block_exp == nullptr)
            std::cout << "  " << "ret" << std::endl;
        else {
            std::string result_var = Visit_AST((ExpAST*)(stmt->block_exp.get()));
            std::cout << "  " << "ret " << result_var << std::endl;
        }
        std::string other_label = "%other_" + std::to_string(other_cnt++);
        std::cout << other_label << ":" << std::endl;
    }
    else if (stmt->type == "lval") {
        std::string result_var = Visit_AST((ExpAST*)(stmt->block_exp.get()));
        std::variant<int, std::string> value = look_up_symbol_tables(stmt->l_val);
        assert(value.index() == 1);
        std::cout << "  " << "store " << result_var << ", " << std::get<1>(value) << std::endl;
    }
    else if (stmt->type == "exp") {
        if (stmt->block_exp != nullptr)
            Visit_AST((ExpAST*)(stmt->block_exp.get()));
    }
    else if (stmt->type == "block")
        Visit_AST((BlockAST*)(stmt->block_exp.get()));
    else
        assert(false);
}

void Visit_AST(const StmtAST *stmt) {
    if (stmt->type == "simple")
        Visit_AST((SimpleStmtAST*)(stmt->exp_simple.get()));
    else if (stmt->type == "if") {
        std::string if_result = Visit_AST((ExpAST*)(stmt->exp_simple.get()));
        std::string label_then = "%then_" + std::to_string(if_else_cnt);
        std::string label_end = "%end_" + std::to_string(if_else_cnt);
        if_else_cnt++;
        std::cout << "  " << "br " << if_result << ", " << label_then << ", " << label_end << std::endl;
        std::cout << label_then << ":" << std::endl;
        Visit_AST((StmtAST*)(stmt->if_stmt.get()));
        std::cout << "  " << "jump " << label_end << std::endl;
        std::cout << label_end << ":" << std::endl;
    }
    else if (stmt->type == "ifelse") {
        std::string if_result = Visit_AST((ExpAST*)(stmt->exp_simple.get()));
        std::string label_then = "%then_" + std::to_string(if_else_cnt);
        std::string label_else = "%else_" + std::to_string(if_else_cnt);
        std::string label_end = "%end_" + std::to_string(if_else_cnt);
        if_else_cnt++;
        std::cout << "  " << "br " << if_result << ", " << label_then << ", " << label_else << std::endl;
        std::cout << label_then << ":" << std::endl;
        Visit_AST((StmtAST*)(stmt->if_stmt.get()));
        std::cout << "  " << "jump " << label_end << std::endl;
        std::cout << label_else << ":" << std::endl;
        Visit_AST((StmtAST*)(stmt->else_stmt.get()));
        std::cout << "  " << "jump " << label_end << std::endl;
        std::cout << label_end << ":" << std::endl;
    }
    else
        assert(false);
}

std::string Visit_AST(const ExpAST *exp) {
    std::string result_var = Visit_AST((LOrExpAST*)(exp->lor_exp.get()));
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
    else if (primary_exp->type == "lval") {
        std::variant<int, std::string> value = look_up_symbol_tables(primary_exp->l_val);
        if (value.index() == 0) // const_var
            result_var = std::to_string(std::get<0>(value));
        else { // var
            result_var = "%" + std::to_string(var_cnt++);
            std::cout << "  " << result_var << " = load " << std::get<1>(value) << std::endl;
        }
    }
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

std::string Visit_AST(const RelExpAST *rel_exp) {
    std::string result_var = "";
    if (rel_exp->op == "")
        result_var = Visit_AST((AddExpAST*)(rel_exp->add_exp.get()));
    else {
        std::string left_result = Visit_AST((RelExpAST*)(rel_exp->rel_exp.get()));
        std::string right_result = Visit_AST((AddExpAST*)(rel_exp->add_exp.get()));
        result_var = "%" + std::to_string(var_cnt++);
        if (rel_exp->op == "<")
            std::cout << "  " << result_var << " = lt " << left_result << ", " << right_result << std::endl;
        else if (rel_exp->op == ">")
            std::cout << "  " << result_var << " = gt " << left_result << ", " << right_result << std::endl;
        else if (rel_exp->op == "<=")
            std::cout << "  " << result_var << " = le " << left_result << ", " << right_result << std::endl;
        else if (rel_exp->op == ">=")
            std::cout << "  " << result_var << " = ge " << left_result << ", " << right_result << std::endl;
        else
            assert(false);
    }

    return result_var;
}

std::string Visit_AST(const EqExpAST *eq_exp) {
    std::string result_var = "";
    if (eq_exp->op == "")
        result_var = Visit_AST((RelExpAST*)(eq_exp->rel_exp.get()));
    else {
        std::string left_result = Visit_AST((EqExpAST*)(eq_exp->eq_exp.get()));
        std::string right_result = Visit_AST((RelExpAST*)(eq_exp->rel_exp.get()));
        result_var = "%" + std::to_string(var_cnt++);
        if (eq_exp->op == "==")
            std::cout << "  " << result_var << " = eq " << left_result << ", " << right_result << std::endl;
        else if (eq_exp->op == "!=")
            std::cout << "  " << result_var << " = ne " << left_result << ", " << right_result << std::endl;
        else
            assert(false);
    }

    return result_var;
}

std::string Visit_AST(const LAndExpAST *land_exp) {
    std::string result_var = "";
    if (land_exp->op == "")
        result_var = Visit_AST((EqExpAST*)(land_exp->eq_exp.get()));
    else if (land_exp->op == "&&") {
        std::string left_result = Visit_AST((LAndExpAST*)(land_exp->land_exp.get()));
        std::string label_then = "%then_" + std::to_string(if_else_cnt);
        std::string label_else = "%else_" + std::to_string(if_else_cnt);
        std::string label_end = "%end_" + std::to_string(if_else_cnt);
        if_else_cnt++;

        std::string result_var_ptr = "%" + std::to_string(var_cnt++);
        
        std::cout << "  " << result_var_ptr << " = alloc i32" << std::endl;
        std::cout << "  " << "br " << left_result << ", " << label_then << ", " << label_else << std::endl;
        std::cout << label_then << ":" << std::endl;
        std::string temp_result_var = "%" + std::to_string(var_cnt++);
        std::string right_result = Visit_AST((EqExpAST*)(land_exp->eq_exp.get()));
        std::cout << "  " << temp_result_var << " = ne " << right_result << ", 0" << std::endl;
        std::cout << "  " << "store " << temp_result_var << ", " << result_var_ptr << std::endl;
        std::cout << "  " << "jump " << label_end << std::endl;
        std::cout << label_else << ":" << std::endl;
        std::cout << "  " << "store 0, " << result_var_ptr << std::endl;
        std::cout << "  " << "jump " << label_end << std::endl;
        std::cout << label_end << ":" << std::endl;
        result_var = "%" + std::to_string(var_cnt++);
        std::cout << "  " << result_var << " = load " << result_var_ptr << std::endl;
    }
    else
        assert(false);

    return result_var;
}

std::string Visit_AST(const LOrExpAST *lor_exp) {
    std::string result_var = "";
    if (lor_exp->op == "")
        result_var = Visit_AST((LAndExpAST*)(lor_exp->land_exp.get()));
    else if (lor_exp->op == "||") {
        std::string left_result = Visit_AST((LOrExpAST*)(lor_exp->lor_exp.get()));
        std::string label_then = "%then_" + std::to_string(if_else_cnt);
        std::string label_else = "%else_" + std::to_string(if_else_cnt);
        std::string label_end = "%end_" + std::to_string(if_else_cnt);
        if_else_cnt++;

        std::string result_var_ptr = "%" + std::to_string(var_cnt++);

        std::cout << "  " << result_var_ptr << " = alloc i32" << std::endl;
        std::cout << "  " << "br " << left_result << ", " << label_then << ", " << label_else << std::endl;
        std::cout << label_then << ":" << std::endl;
        std::cout << "  " << "store 1, " << result_var_ptr << std::endl;
        std::cout << "  " << "jump " << label_end << std::endl;
        std::cout << label_else << ":" << std::endl;
        std::string temp_result_var = "%" + std::to_string(var_cnt++);
        std::string right_result = Visit_AST((LAndExpAST*)(lor_exp->land_exp.get()));
        std::cout << "  " << temp_result_var << " = ne " << right_result << ", 0" << std::endl;
        std::cout << "  " << "store " << temp_result_var << ", " << result_var_ptr << std::endl;
        std::cout << "  " << "jump " << label_end << std::endl;
        std::cout << label_end << ":" << std::endl;
        result_var = "%" + std::to_string(var_cnt++);
        std::cout << "  " << result_var << " = load " << result_var_ptr << std::endl;
    }
    else
        assert(false);

    return result_var;
}

int Cal_AST(const ExpAST *exp) {
    int result = Cal_AST((LOrExpAST*)(exp->lor_exp.get()));
    return result;
}

int Cal_AST(const UnaryExpAST *unary_exp) {
    int result = 0;
    if (unary_exp->type == "primary")
        result = Cal_AST((PrimaryExpAST*)(unary_exp->exp.get()));
    else if (unary_exp->type == "unary") {
        int temp = Cal_AST((UnaryExpAST*)(unary_exp->exp.get()));

        switch (unary_exp->op[0])
        {
        case '+':
            result = temp;
            break;
        case '-':
            result = -temp;
            break;
        case '!':
            result = !temp;
            break;
        default:
            assert(false);
        }
    }
    else
        assert(false);

    return result;
}

int Cal_AST(const PrimaryExpAST *primary_exp) {
    int result = 0;
    if (primary_exp->type == "exp")
        result = Cal_AST((ExpAST*)(primary_exp->exp.get()));
    else if (primary_exp->type == "number")
        result = primary_exp->number;
    else if (primary_exp->type == "lval") {
        std::variant<int, std::string> value = look_up_symbol_tables(primary_exp->l_val);
        assert(value.index() == 0);
        result = std::get<0>(value);
    }
    else
        assert(false);

    return result;
}

int Cal_AST(const MulExpAST *mul_exp) {
    int result = 0;
    if (mul_exp->op == "")
        result = Cal_AST((UnaryExpAST*)(mul_exp->unary_exp.get()));
    else {
        int left_result = Cal_AST((MulExpAST*)(mul_exp->mul_exp.get()));
        int right_result = Cal_AST((UnaryExpAST*)(mul_exp->unary_exp.get()));
        switch (mul_exp->op[0])
        {
        case '*':
            result = left_result * right_result;
            break;
        case '/':
            result = left_result / right_result;
            break;
        case '%':
            result = left_result % right_result;
            break;
        default:
            assert(false);
            break;
        }
    }

    return result;
}

int Cal_AST(const AddExpAST *add_exp) {
    int result = 0;
    if (add_exp->op == "")
        result = Cal_AST((MulExpAST*)(add_exp->mul_exp.get()));
    else {
        int left_result = Cal_AST((AddExpAST*)(add_exp->add_exp.get()));
        int right_result = Cal_AST((MulExpAST*)(add_exp->mul_exp.get()));
        switch (add_exp->op[0])
        {
        case '+':
            result = left_result + right_result;
            break;
        case '-':
            result = left_result - right_result;
            break;
        default:
            assert(false);
            break;
        }
    }

    return result;
}

int Cal_AST(const RelExpAST *rel_exp) {
    int result = 0;
    if (rel_exp->op == "")
        result = Cal_AST((AddExpAST*)(rel_exp->add_exp.get()));
    else {
        int left_result = Cal_AST((RelExpAST*)(rel_exp->rel_exp.get()));
        int right_result = Cal_AST((AddExpAST*)(rel_exp->add_exp.get()));
        if (rel_exp->op == "<")
            result = (left_result < right_result);
        else if (rel_exp->op == ">")
            result = (left_result > right_result);
        else if (rel_exp->op == "<=")
            result = (left_result <= right_result);
        else if (rel_exp->op == ">=")
            result = (left_result >= right_result);
        else
            assert(false);
    }

    return result;
}

int Cal_AST(const EqExpAST *eq_exp) {
    int result = 0;
    if (eq_exp->op == "")
        result = Cal_AST((RelExpAST*)(eq_exp->rel_exp.get()));
    else {
        int left_result = Cal_AST((EqExpAST*)(eq_exp->eq_exp.get()));
        int right_result = Cal_AST((RelExpAST*)(eq_exp->rel_exp.get()));
        if (eq_exp->op == "==")
            result = (left_result == right_result);
        else if (eq_exp->op == "!=")
            result = (left_result != right_result);
        else
            assert(false);
    }

    return result;
}

int Cal_AST(const LAndExpAST *land_exp) {
    int result = 0;
    if (land_exp->op == "")
        result = Cal_AST((EqExpAST*)(land_exp->eq_exp.get()));
    else if (land_exp->op == "&&") {
        int left_result = Cal_AST((LAndExpAST*)(land_exp->land_exp.get()));
        if (left_result == 0)
            return 0;
        result = (Cal_AST((EqExpAST*)(land_exp->eq_exp.get())) != 0);
    }
    else
        assert(false);

    return result;
}

int Cal_AST(const LOrExpAST *lor_exp) {
    int result = 1;
    if (lor_exp->op == "")
        result = Cal_AST((LAndExpAST*)(lor_exp->land_exp.get()));
    else if (lor_exp->op == "||") {
        int left_result = Cal_AST((LOrExpAST*)(lor_exp->lor_exp.get()));
        if (left_result)
            return 1;
        result = (Cal_AST((LAndExpAST*)(lor_exp->land_exp.get())) != 0);
    }
    else
        assert(false);

    return result;
}

void Visit_AST(const DeclAST *decl) {
    if (decl->type == "const_decl")
        Visit_AST((ConstDeclAST*)(decl->decl.get()));
    else if (decl->type == "var_decl")
        Visit_AST((VarDeclAST*)(decl->decl.get()));
    else
        assert(false);
}

void Visit_AST(const BlockItemAST *block_item) {
    if (block_item->type == "decl")
        Visit_AST((DeclAST*)(block_item->content.get()));
    else if (block_item->type == "stmt")
        Visit_AST((StmtAST*)(block_item->content.get()));
    else
        assert(false);
}

void Visit_AST(const ConstDeclAST *const_decl) {
    assert(const_decl->b_type == "int"); // Only support int at present
    int size = const_decl->const_def_list.size();
    for (int i = 0; i < size; ++i)
        Visit_AST((ConstDefAST*)(const_decl->const_def_list[i].get()));
}

void Visit_AST(const ConstDefAST *const_def) {
    int i = symbol_tables.size() - 1;
    symbol_tables[i][const_def->ident] = Visit_AST((ConstInitValAST*)(const_def->const_init_val.get()));
}

int Visit_AST(const ConstInitValAST *const_init_val) {
    return Cal_AST((ConstExpAST*)(const_init_val->const_exp.get()));
}

int Cal_AST(const ConstExpAST *const_exp) {
    return Cal_AST((ExpAST*)(const_exp->exp.get()));
}

void Visit_AST(const VarDeclAST *var_decl) {
    assert(var_decl->b_type == "int"); // Only support int type
    int size = var_decl->var_def_list.size();
    for (int i = 0; i < size; ++i)
        Visit_AST((VarDefAST*)(var_decl->var_def_list[i].get()));
}

void Visit_AST(const VarDefAST *var_def) {
    std::string var_name = "@" + var_def->ident;
    std::string name = var_name + "_" + std::to_string(var_names[var_name]++);
    std::cout << "  " << name << " = alloc i32" << std::endl;
    int i = symbol_tables.size() - 1;
    symbol_tables[i][var_def->ident] = name;
    if (var_def->has_init_val) {
        std::string value = Visit_AST((InitValAST*)(var_def->init_val.get()));
        std::cout << "  store " << value << ", " << name << std::endl;
    }
}

std::string Visit_AST(const InitValAST *init_val) {
    return Visit_AST((ExpAST*)(init_val->exp.get()));
}