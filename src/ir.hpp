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
std::map<std::string, std::string> function_table;
std::map<std::string, int> var_names;
std::map<std::string, int> is_ptr;
std::map<std::string, int> is_arr;
std::map<std::string, int> arr_dim;
int if_else_cnt = 0;
int other_cnt = 0;
int while_cnt = 0;
int in_call_func = 0;
std::string while_entry = "";
std::string while_end = "";

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
void Visit_AST(const InitValAST *init_val, std::vector<std::string> &array, std::vector<int> sizes);
std::string Visit_AST(const FuncFParamAST* func_f_param);
void Visit_AST(const ConstInitValAST *const_init_val, int *array, std::vector<int> sizes);
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
void Visit_Global_Decl(const DeclAST *decl);
void print_array(int *array, std::vector<int>sizes, int &pos);
void print_array(std::vector<std::string> array, std::vector<int>sizes, int &pos);

void print_array(int *array, std::vector<int>sizes, int &pos) {
    int sizes_size = sizes.size();
    if (sizes_size == 1) {
        int size = sizes[0];
        std::cout << "{";
        for (int k = 0; k < size; ++k) {
            std::cout << std::to_string(*(array+pos));
            pos++;
            if (k != size - 1)
                std::cout << ", ";
        }
        std::cout << "}";
    }
    else {
        int size = sizes[sizes_size - 1];
        sizes.pop_back();
        std::cout << "{";
        for (int k = 0; k < size; ++k) {
            print_array(array, sizes, pos);
            if (k != size - 1)
                std::cout << ", ";
        }
        std::cout << "}";
    }
}

void print_array(std::vector<std::string> array, std::vector<int>sizes, int &pos) {
    int sizes_size = sizes.size();
    if (sizes_size == 1) {
        int size = sizes[0];
        std::cout << "{";
        for (int k = 0; k < size; ++k) {
            std::cout << array[pos];
            pos++;
            if (k != size - 1)
                std::cout << ", ";
        }
        std::cout << "}";
    }
    else {
        int size = sizes[sizes_size - 1];
        sizes.pop_back();
        std::cout << "{";
        for (int k = 0; k < size; ++k) {
            print_array(array, sizes, pos);
            if (k != size - 1)
                std::cout << ", ";
        }
        std::cout << "}";
    }
}

void Visit_Global_Decl(const DeclAST *decl) {
    if (decl->type == "const_decl") {
        ConstDeclAST *const_decl = (ConstDeclAST*)(decl->decl.get());
        int size_ = const_decl->const_def_list.size();
        for (int i = 0; i < size_; ++i) {
            ConstDefAST *const_def = (ConstDefAST*)(const_decl->const_def_list[i].get());
            if (const_def->const_exps.size() == 0)
                Visit_AST(const_def);
            else {
                std::string arr_name = "@" + const_def->ident;
                std::string name = arr_name + "_" + std::to_string(var_names[arr_name]++);
                int index = symbol_tables.size() - 1;
                symbol_tables[index][const_def->ident] = name;
                is_arr[name] = 1;
                ConstInitValAST *init_val = (ConstInitValAST *)(const_def->const_init_val.get());
                int sizes_size = const_def->const_exps.size(), len = 1;
                arr_dim[name] = sizes_size;
                std::vector<int> sizes;
                std::string alloc_str;
                for (int j = sizes_size - 1; j >= 0; --j) {
                    int size = Cal_AST((ConstExpAST*)(const_def->const_exps[j].get()));
                    len *= size;
                    sizes.push_back(size);
                    if (j == sizes_size - 1)
                        alloc_str = "[i32, " + std::to_string(size) + "]";
                    else
                        alloc_str = "[" + alloc_str + ", " + std::to_string(size) + "]";
                }
                std::cout << "global " << name << " = alloc " << alloc_str << ", ";
                int *array = (int *)malloc(sizeof(int) * len);
                Visit_AST(init_val, array, sizes);
                int temp_pos = 0;
                print_array(array, sizes, temp_pos);
                std::cout << std::endl;
                free(array);
            }
        }
    }
    else if (decl->type == "var_decl") {
        VarDeclAST *var_decl = (VarDeclAST*)(decl->decl.get());
        assert(var_decl->b_type == "int"); // Only support int type
        int size_ = var_decl->var_def_list.size();
        for (int i = 0; i < size_; ++i) {
            VarDefAST *var_def = (VarDefAST*)(var_decl->var_def_list[i].get());
            
            std::string var_name = "@" + var_def->ident;
            std::string name = var_name + "_" + std::to_string(var_names[var_name]++);
            int index = symbol_tables.size() - 1;
            symbol_tables[index][var_def->ident] = name;

            if (var_def->const_exps.size() == 0) { // var    
                std::cout << "global " << name << " = alloc i32, ";
                if (var_def->init_val != nullptr) {
                    InitValAST *init_val = (InitValAST*)(var_def->init_val.get());
                    std::string value = Visit_AST((ExpAST*)(init_val->exp.get()));
                    assert(value[0] != '@' && value[0] != '%');
                    if (value != "0")
                        std::cout << value << std::endl;
                    else
                        std::cout << "zeroinit" << std::endl;
                }
                else
                    std::cout << "zeroinit" << std::endl;
            }
            else { // array
                is_arr[name] = 1;
                int sizes_size = var_def->const_exps.size(), len = 1;
                arr_dim[name] = sizes_size;
                std::vector<int> sizes;
                std::string alloc_str;
                for (int j = sizes_size - 1; j >= 0; --j) {
                    int size = Cal_AST((ConstExpAST*)(var_def->const_exps[j].get()));
                    len *= size;
                    sizes.push_back(size);
                    if (j == sizes_size - 1)
                        alloc_str = "[i32, " + std::to_string(size) + "]";
                    else
                        alloc_str = "[" + alloc_str + ", " + std::to_string(size) + "]";
                }
                std::cout << "global " << name << " = alloc " << alloc_str;
                if (var_def->init_val != nullptr) {
                    std::cout << ", ";
                    InitValAST *init_val = (InitValAST*)(var_def->init_val.get());
                    std::vector<std::string> array;
                    Visit_AST(init_val, array, sizes);
                    int temp_pos = 0;
                    print_array(array, sizes, temp_pos);
                    std::cout << std::endl;
                }
                else
                    std::cout << ", zeroinit" << std::endl;
            }
        }
    }
    else
        assert(false);
}

std::variant<int, std::string> look_up_symbol_tables(std::string ident) {
    int size = symbol_tables.size();
    for (int i = size-1; i >= 0; --i) {
        if (symbol_tables[i].count(ident))
            return symbol_tables[i][ident];
    }

    assert(false);
    return -1;
}

void Visit_AST(const CompUnitAST *comp_unit) {
    std::cout << "decl @getint(): i32" << std::endl;
    std::cout << "decl @getch(): i32" << std::endl;
    std::cout << "decl @getarray(*i32): i32" << std::endl;
    std::cout << "decl @putint(i32)" << std::endl;
    std::cout << "decl @putch(i32)" << std::endl;
    std::cout << "decl @putarray(i32, *i32)" << std::endl;
    std::cout << "decl @starttime()" << std::endl;
    std::cout << "decl @stoptime()" << std::endl;
    std::cout << std::endl;
    function_table["getint"] = "int";
    function_table["getch"] = "int";
    function_table["getarray"] = "int";
    function_table["putint"] = "void";
    function_table["putch"] = "void";
    function_table["putarray"] = "void";
    function_table["starttime"] = "void";
    function_table["stoptime"] = "void";

    std::map<std::string, std::variant<int, std::string> > symbol_table;
    symbol_tables.push_back(symbol_table);
    int size = comp_unit->global_decl_list.size();
    for (int i = 0; i < size; ++i)
        Visit_Global_Decl((DeclAST*)(comp_unit->global_decl_list[i].get()));

    size = comp_unit->func_def_list.size();
    for (int i = 0; i < size; ++i) {
        Visit_AST((FuncDefAST*)(comp_unit->func_def_list[i].get()));
        std::cout << std::endl;
    }
    symbol_tables.pop_back();
}

void Visit_AST(const FuncDefAST *func_def) {
    int old_var_cnt = var_cnt, old_if_else_cnt = if_else_cnt;
    int old_other_cnt = other_cnt, old_while_cnt = while_cnt;
    std::map<std::string, int> old_var_names = var_names;

    // redir
    std::stringstream ir_ss;
    std::streambuf *buffer = std::cout.rdbuf();
    std::cout.rdbuf(ir_ss.rdbuf());

    std::map<std::string, std::variant<int, std::string> > symbol_table;
    symbol_tables.push_back(symbol_table);

    std::cout << "fun @" << func_def->ident;
    std::cout << "(";
    int size = func_def->func_f_params.size();
    std::vector<std::string> temp_v;
    for (int i = 0; i < size; ++i) {
        std::string temp = Visit_AST((FuncFParamAST*)(func_def->func_f_params[i].get()));
        if (i != size-1)
            std::cout << ", ";
        temp_v.push_back(temp);
    }
    std::cout << ")";
    std::string type = func_def->func_type;
    function_table[func_def->ident] = type;
    if (type == "int")
        std::cout << ": i32 {" << std::endl;
    else if (type == "void")
        std::cout << " {" << std::endl;
    else
        assert(false);
    std::cout << "%entry:" << std::endl; // Blocks will have their names in the future
    size = temp_v.size();
    for (int i = 0; i < size; ++i)
        std::cout << temp_v[i];
    Visit_AST((BlockAST*)(func_def->block.get()));

    std::string ir_str = ir_ss.str(), last_line = "";
    int pt = ir_str.length() - 2;
    while (ir_str[pt] != '\n')
        last_line = ir_str[pt--] + last_line;
    if (last_line.substr(0, 6) == "%other") // deal with empty ret block
        ir_str = ir_str.substr(0, pt+1);

    pt = ir_str.length() - 2; last_line = "";
    while (ir_str[pt] != '\n')
        last_line = ir_str[pt--] + last_line;
    if (last_line.substr(0, 5) != "  ret") { // deal with empty ret block
        assert(type == "void");
        ir_str += "  ret\n";
    }
    std::cout.rdbuf(buffer);

    std::cout << ir_str;
    std::cout << "}" << std::endl;

    symbol_tables.pop_back();
    var_cnt = old_var_cnt; if_else_cnt = old_if_else_cnt;
    other_cnt = old_other_cnt; while_cnt = old_while_cnt;
    var_names = old_var_names;
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
        LValAST *l_val = (LValAST *)(stmt->l_val.get());
        std::variant<int, std::string> value = look_up_symbol_tables(l_val->ident);
        assert(value.index() == 1);
        int size = l_val->exps.size();
        if (size == 0)
            std::cout << "  " << "store " << result_var << ", " << std::get<1>(value) << std::endl;
        else { // array
            std::string ptr_val, add = std::get<1>(value);
            if (is_ptr[add]) {
                ptr_val = "%" + std::to_string(var_cnt++);
                std::cout << "  " << ptr_val << " = load " << add << std::endl;
                add = ptr_val;
                for (int i = 0; i < size; ++i) {
                    std::string index = Visit_AST((ExpAST*)(l_val->exps[i].get()));
                    ptr_val = "%" + std::to_string(var_cnt++);
                    if (i == 0)
                        std::cout << "  " << ptr_val << " = getptr " << add << ", " << index << std::endl;
                    else
                        std::cout << "  " << ptr_val << " = getelemptr " << add << ", " << index << std::endl;
                    add = ptr_val;
                }
            }
            else {
                for (int i = 0; i < size; ++i) {
                    std::string index = Visit_AST((ExpAST*)(l_val->exps[i].get()));
                    ptr_val = "%" + std::to_string(var_cnt++);
                    std::cout << "  " << ptr_val << " = getelemptr " << add << ", " << index << std::endl;
                    add = ptr_val;
                }
            }
            std::cout << "  " << "store " << result_var << ", " << ptr_val << std::endl;
        }
    }
    else if (stmt->type == "exp") {
        if (stmt->block_exp != nullptr)
            Visit_AST((ExpAST*)(stmt->block_exp.get()));
    }
    else if (stmt->type == "block")
        Visit_AST((BlockAST*)(stmt->block_exp.get()));
    else if (stmt->type == "break") {
        assert(while_end != "");
        std::string other_label = while_end + "_break";
        std::cout << "  " << "jump " << while_end << std::endl;
        std::cout << other_label << ":" << std::endl;
    }
    else if (stmt->type == "continue") {
        assert(while_entry != "");
        std::string other_label = while_end + "_continue";
        std::cout << "  " << "jump " << while_entry << std::endl;
        std::cout << other_label << ":" << std::endl;
    }
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
    else if (stmt->type == "while") {
        std::string entry_label = "%while_entry_" + std::to_string(while_cnt);
        std::string body_label = "%while_body_" + std::to_string(while_cnt);
        std::string end_label = "%while_end_" + std::to_string(while_cnt);
        
        std::string old_entry = while_entry, old_end = while_end;
        while_entry = entry_label; while_end = end_label;

        while_cnt++;
        std::cout << "  " << "jump " << entry_label << std::endl;
        std::cout << entry_label << ":" << std::endl;
        std::string cond = Visit_AST((ExpAST*)(stmt->exp_simple.get()));
        std::cout << "  " << "br " << cond << ", " << body_label << ", " << end_label << std::endl;
        std::cout << body_label << ":" << std::endl;
        Visit_AST((StmtAST*)(stmt->while_stmt.get()));
        std::cout << "  " << "jump " << entry_label << std::endl;
        std::cout << end_label << ":" << std::endl;

        while_entry = old_entry; while_end = old_end;
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
    else if (unary_exp->type == "call") {
        assert(function_table.count(unary_exp->ident));
        in_call_func = 1;
        std::vector<std::string> params;
        int size = unary_exp->func_r_params.size();
        for (int i = 0; i < size; ++i) {
            std::string param = Visit_AST((ExpAST*)(unary_exp->func_r_params[i].get()));
            params.push_back(param);
        }
        std::string result_var = "";
        if (function_table[unary_exp->ident] == "int") {
            result_var = "%" + std::to_string(var_cnt++);
            std::cout << "  " << result_var << " = call @" + unary_exp->ident << "(";
        }
        else if (function_table[unary_exp->ident] == "void")
            std::cout << "  " << "call @" + unary_exp->ident << "(";
        else
            assert(false);
        size = params.size();
        for (int i = 0; i < size; ++i) {
            std::cout << params[i];
            if (i != size - 1)
                std::cout << ", ";
        }
        std::cout << ")" << std::endl;
        in_call_func = 0;
        return result_var;
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
        LValAST *l_val = (LValAST*)(primary_exp->l_val.get());
        std::variant<int, std::string> value = look_up_symbol_tables(l_val->ident);
        if (value.index() == 0) // const_var
            result_var = std::to_string(std::get<0>(value));
        else { // var
            result_var = "%" + std::to_string(var_cnt++);
            int size = l_val->exps.size();
            if (size == 0) {
                std::string add = std::get<1>(value);
                if (in_call_func && is_arr[add])
                    std::cout << "  " << result_var << " = getelemptr " << add << ", 0" << std::endl;
                else
                    std::cout << "  " << result_var << " = load " << std::get<1>(value) << std::endl;
            }
            else { // array
                std::string ptr_val, add = std::get<1>(value);
                if (is_ptr[add]) {
                    ptr_val = "%" + std::to_string(var_cnt++);
                    std::cout << "  " << ptr_val << " = load " << add << std::endl;
                    add = ptr_val;
                    for (int i = 0; i < size; ++i) {
                        std::string index = Visit_AST((ExpAST*)(l_val->exps[i].get()));
                        ptr_val = "%" + std::to_string(var_cnt++);
                        if (i == 0)
                            std::cout << "  " << ptr_val << " = getptr " << add << ", " << index << std::endl;
                        else
                            std::cout << "  " << ptr_val << " = getelemptr " << add << ", " << index << std::endl;
                        add = ptr_val;
                    }
                }
                else {
                    for (int i = 0; i < size; ++i) {
                        std::string index = Visit_AST((ExpAST*)(l_val->exps[i].get()));
                        ptr_val = "%" + std::to_string(var_cnt++);
                        std::cout << "  " << ptr_val << " = getelemptr " << add << ", " << index << std::endl;
                        add = ptr_val;
                    }
                }
                if (in_call_func && (arr_dim[std::get<1>(value)] > size))
                    std::cout << "  " << result_var << " = getelemptr " << ptr_val << ", 0" << std::endl;
                // if (in_call_func)
                //     std::cout << "  " << result_var << " = getelemptr " << ptr_val << ", 0" << std::endl;
                else
                    std::cout << "  " << result_var << " = load " << ptr_val << std::endl;
            }
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
        LValAST *l_val = (LValAST *)(primary_exp->l_val.get());
        std::variant<int, std::string> value = look_up_symbol_tables(l_val->ident);
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
    int index = symbol_tables.size() - 1;
    ConstInitValAST *init_val = (ConstInitValAST *)(const_def->const_init_val.get());
    if (const_def->const_exps.size() == 0) { // var
        assert(init_val->const_exp != nullptr);
        int val = Cal_AST((ConstExpAST*)(init_val->const_exp.get()));
        symbol_tables[index][const_def->ident] = val;
    }
    else { // array
        std::string arr_name = "@" + const_def->ident;
        std::string name = arr_name + "_" + std::to_string(var_names[arr_name]++);
        symbol_tables[index][const_def->ident] = name;
        is_arr[name] = 1;
        int sizes_size = const_def->const_exps.size(), len = 1;
        arr_dim[name] = sizes_size;
        std::vector<int> sizes;
        std::string alloc_str;
        for (int i = sizes_size - 1; i >= 0; --i) {
            int size = Cal_AST((ConstExpAST*)(const_def->const_exps[i].get()));
            len *= size;
            sizes.push_back(size);
            if (i == sizes_size - 1)
                alloc_str = "[i32, " + std::to_string(size) + "]";
            else
                alloc_str = "[" + alloc_str + ", " + std::to_string(size) + "]";
        }
        std::cout << "  " << name << " = alloc " << alloc_str << std::endl;
        int *array = (int *)malloc(sizeof(int) * len);
        Visit_AST(init_val, array, sizes);
        for (int i = 0; i < len; ++i) {
            int index, temp_len = len, pos = i;
            std::string ptr_val, add = name;
            for (int j = sizes_size - 1; j >= 0; --j) {
                temp_len /= sizes[j];
                index = pos / temp_len;
                pos -= index * temp_len;
                ptr_val = "%" + std::to_string(var_cnt++);
                std::cout << "  " << ptr_val << " = getelemptr " << add << ", " << std::to_string(index) << std::endl;
                add = ptr_val;
            }
            std::cout << "  " << "store " << std::to_string(*(array+i)) << ", " << ptr_val << std::endl;
        }
        free(array);
    }
}

void Visit_AST(const ConstInitValAST *const_init_val, int *array, std::vector<int> sizes) {
    int sizes_size = sizes.size(), pos = 0;
    assert(const_init_val->const_exp == nullptr);
    int size = const_init_val->const_init_vals.size();
    for (int i = 0; i < size; ++i) {
        ConstInitValAST *init_val = (ConstInitValAST *)(const_init_val->const_init_vals[i].get());
        if (init_val->const_exp != nullptr) { // is a integer
            *(array+pos) = Cal_AST((ConstExpAST*)(init_val->const_exp.get()));
            pos++;
        }
        else { // is an aggregate
            std::vector<int> new_sizes;
            int len = 1;
            for (int j = 0; j < sizes_size - 1; ++j) { // sizes中 0 是最后一个维度
                len *= sizes[j];
                if (pos % len == 0)
                    new_sizes.push_back(sizes[j]);
                else {
                    len /= sizes[j];
                    break;
                }
            }
            Visit_AST(init_val, array+pos, new_sizes);
            pos += len;
        }
    }
    int len = 1;
    for (int i = 0; i < sizes_size; ++i)
        len *= sizes[i];
    for ( ; pos < len; ++pos)
        *(array+pos) = 0;
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
    int index = symbol_tables.size() - 1;
    if (var_def->const_exps.size() == 0) {
        std::string var_name = "@" + var_def->ident;
        std::string name = var_name + "_" + std::to_string(var_names[var_name]++);
        std::cout << "  " << name << " = alloc i32" << std::endl;
        symbol_tables[index][var_def->ident] = name;
        if (var_def->init_val != nullptr) {
            InitValAST *init_val = (InitValAST*)(var_def->init_val.get());
            std::string val = Visit_AST((ExpAST*)(init_val->exp.get()));
            std::cout << "  store " << val << ", " << name << std::endl;
        }
    }
    else { // array
        std::string arr_name = "@" + var_def->ident;
        std::string name = arr_name + "_" + std::to_string(var_names[arr_name]++);
        symbol_tables[index][var_def->ident] = name;
        is_arr[name] = 1;
        int sizes_size = var_def->const_exps.size(), len = 1;
        arr_dim[name] = sizes_size;
        std::vector<int> sizes;
        std::string alloc_str;
        for (int i = sizes_size - 1; i >= 0; --i) {
            int size = Cal_AST((ConstExpAST*)(var_def->const_exps[i].get()));
            len *= size;
            sizes.push_back(size);
            if (i == sizes_size - 1)
                alloc_str = "[i32, " + std::to_string(size) + "]";
            else
                alloc_str = "[" + alloc_str + ", " + std::to_string(size) + "]";
        }
        std::cout << "  " << name << " = alloc " << alloc_str << std::endl;
        if (var_def->init_val != nullptr) {
            InitValAST *init_val = (InitValAST*)(var_def->init_val.get());
            std::vector<std::string> array;
            Visit_AST(init_val, array, sizes);
            for (int i = 0; i < len; ++i) {
                int index, temp_len = len, pos = i;
                std::string ptr_val, add = name;
                for (int j = sizes_size - 1; j >= 0; --j) {
                    temp_len /= sizes[j];
                    index = pos / temp_len;
                    pos -= index * temp_len;
                    ptr_val = "%" + std::to_string(var_cnt++);
                    std::cout << "  " << ptr_val << " = getelemptr " << add << ", " << std::to_string(index) << std::endl;
                    add = ptr_val;
                }
                std::cout << "  " << "store " << array[i] << ", " << ptr_val << std::endl;
            }
        }
    }
}

void Visit_AST(const InitValAST *init_val, std::vector<std::string> &array, std::vector<int> sizes) {
    int sizes_size = sizes.size(), pos = 0;
    assert(init_val->exp == nullptr);
    int size = init_val->init_vals.size();
    for (int i = 0; i < size; ++i) {
        InitValAST *init_val_ = (InitValAST*)(init_val->init_vals[i].get());
        if (init_val_->exp != nullptr) {
            array.push_back(Visit_AST((ExpAST*)(init_val_->exp.get())));
            pos++;
        }
        else {
            std::vector<int> new_sizes;
            int len = 1;
            for (int j = 0; j < sizes_size - 1; ++j) { // sizes中 0 是最后一个维度
                len *= sizes[j];
                if (pos % len == 0)
                    new_sizes.push_back(sizes[j]);
                else {
                    len /= sizes[j];
                    break;
                }
            }
            Visit_AST(init_val_, array, new_sizes);
            pos += len;
        }
    }
    int len = 1;
    for (int i = 0; i < sizes_size; ++i)
        len *= sizes[i];
    for ( ; pos < len; ++pos)
        array.push_back("0");
}

std::string Visit_AST(const FuncFParamAST* func_f_param) {
    std::string return_insts;
    if (func_f_param->is_array) {
        int i = symbol_tables.size() - 1, sizes_size = func_f_param->const_exps.size();
        std::string display_name = "@param_" + func_f_param->ident, result_var = "%param_" + func_f_param->ident;
        std::string alloc_str = "i32";
        for (int i = sizes_size - 1; i >= 0; --i) {
            int size = Cal_AST((ConstExpAST*)(func_f_param->const_exps[i].get()));
            alloc_str = "[" + alloc_str + ", " + std::to_string(size) + "]";
        }
        std::cout << display_name << ": *" << alloc_str;
        symbol_tables[i][func_f_param->ident] = result_var;
        is_ptr[result_var] = 1;
        arr_dim[result_var] = sizes_size + 1;
        return_insts = "  " + result_var + " = alloc *" + alloc_str + "\n  store " + display_name + ", " + result_var + "\n";
    }
    else {
        int i = symbol_tables.size() - 1;
        assert(func_f_param->b_type == "int");
        std::string display_name = "@param_" + func_f_param->ident, result_var = "%param_" + func_f_param->ident;
        std::cout << display_name << ": i32";
        symbol_tables[i][func_f_param->ident] = result_var;
        return_insts = "  " + result_var + " = alloc i32\n  store " + display_name + ", " + result_var + "\n";
    }
    
    return return_insts;
}