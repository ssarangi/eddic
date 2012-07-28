//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include <iostream>

#include "ast/Printer.hpp"

#include "variant.hpp"
#include "assert.hpp"
#include "VisitorUtils.hpp"
#include "Variable.hpp"

using namespace eddic;

namespace {
    
std::string toStringType(ast::Type type){
    if(auto* ptr = boost::get<ast::SimpleType>(&type)){
        return ptr->type;
    } else if(auto* ptr = boost::get<ast::ArrayType>(&type)){
        return ptr->type + "[]";
    } else {
        ASSERT_PATH_NOT_TAKEN("Unhandled type");
    }
}

struct DebugVisitor : public boost::static_visitor<> {
    mutable int level = 0;

    std::string indent() const {
        std::string acc = "";
        for(int i = 0; i < level; ++i){
            acc += "    ";
        }
        return acc;
    }

    template<typename Container>
    void print_each_sub(Container& container) const {
        level++;
        visit_each(*this, container);    
        level--;
    }

    template<typename Container>
    void print_each_sub(Container& container, const std::string& title) const {
        std::cout << indent() << title << std::endl; 
        print_each_sub(container);
    }

    template<typename Container>
    void print_each_sub_non_variant(Container& container) const {
        level++;
        visit_each_non_variant(*this, container);    
        level--;
    }

    template<typename Container>
    void print_each_sub_non_variant(Container& container, const std::string& title) const {
        std::cout << indent() << title << std::endl; 
        print_each_sub_non_variant(container);
    }

    template<typename Container>
    void print_sub(Container& container) const {
        level++;
        visit(*this, container);    
        level--;
    }

    template<typename Container>
    void print_sub(Container& container, const std::string& title) const {
        std::cout << indent() << title << std::endl; 
        print_sub(container);
    }

    void operator()(ast::SourceFile& program) const {
        print_each_sub(program.Content->blocks, "SourceFile");
    }

    void operator()(ast::Import& import) const {
        std::cout << indent() << "include \"" << import.file << "\"" << std::endl;
    }

    void operator()(ast::StandardImport& import) const {
        std::cout << indent() << "include <" << import.header << ">" << std::endl;
    }

    void operator()(ast::FunctionDeclaration& declaration) const {
        std::cout << indent() << "Function " << declaration.Content->functionName << std::endl; 
        
        std::cout << indent() << "Parameters:" << std::endl; 
        level++;
        for(auto param : declaration.Content->parameters){
            std::cout << indent() << param.parameterName << std::endl; 
        }
        level--;
        
        print_each_sub(declaration.Content->instructions, "Instructions:");
    }
    
    void operator()(ast::Constructor& declaration) const {
        std::cout << indent() << "Constructor" << std::endl; 
        
        std::cout << indent() << "Parameters:" << std::endl; 
        level++;
        for(auto param : declaration.Content->parameters){
            std::cout << indent() << param.parameterName << std::endl; 
        }
        level--;
        
        print_each_sub(declaration.Content->instructions, "Instructions:");
    }
    
    void operator()(ast::Destructor& declaration) const {
        std::cout << indent() << "Destructor" << std::endl; 
        print_each_sub(declaration.Content->instructions, "Instructions:");
    }

    void operator()(ast::Struct& struct_) const {
        std::cout << indent() << "Structure declaration: " << struct_.Content->name << std::endl; 
        level++;
        
        print_each_sub_non_variant(struct_.Content->members, "Members");
        print_each_sub_non_variant(struct_.Content->constructors, "Constructors");
        print_each_sub_non_variant(struct_.Content->destructors, "Destructors");
        print_each_sub_non_variant(struct_.Content->functions, "Functions");
        
        level--;
    }

    void operator()(ast::MemberDeclaration& declaration) const {
        std::cout << indent() << toStringType(declaration.Content->type)  << " " << declaration.Content->name << std::endl;
    }

    void operator()(ast::GlobalVariableDeclaration&) const {
        std::cout << indent() << "Global Variable" << std::endl; 
    }

    void operator()(ast::GlobalArrayDeclaration&) const {
        std::cout << indent() << "Global Array" << std::endl; 
    }
    
    void operator()(ast::New& new_) const {
        std::cout << indent() << "New " << toStringType(new_.Content->type) << std::endl; 
    }
    
    void operator()(ast::Delete& delete_) const {
        std::cout << indent() << "Delete " << delete_.Content->variable_name << std::endl; 
    }

    void operator()(ast::For& for_) const {
        print_each_sub(for_.Content->instructions, "For");
    }

    void operator()(ast::Foreach& for_) const {
        print_each_sub(for_.Content->instructions, "Foreach");
    }

    void operator()(ast::ForeachIn& for_) const {
        print_each_sub(for_.Content->instructions, "Foreach In");
    }

    void operator()(ast::Switch& switch_) const {
        std::cout << indent() << "Switch " << std::endl; 

        ++level;
        
        print_sub(switch_.Content->value, "Value");
        
        for(auto& case_ : switch_.Content->cases){
            visit_non_variant(*this, case_);
        }
        
        if(switch_.Content->default_case){
            visit_non_variant(*this, *switch_.Content->default_case);
        }
        
        --level;
    }

    void operator()(ast::SwitchCase& switch_case) const {
        std::cout << indent() << "Case" << std::endl; 

        ++level;
        
        print_sub(switch_case.value, "Value");
        print_each_sub(switch_case.instructions, "Instructions");
        
        --level;
    }

    void operator()(ast::DefaultCase& default_case) const {
        print_each_sub(default_case.instructions, "Default Case");
    }

    void operator()(ast::While& while_) const {
        std::cout << indent() << "While" << std::endl; 
        print_sub(while_.Content->condition, "Condition:");
        print_each_sub(while_.Content->instructions, "Instructions:");
    }

    void operator()(ast::DoWhile& while_) const {
        std::cout << indent() << "Do while" << std::endl; 
        print_sub(while_.Content->condition, "Condition:");
        print_each_sub(while_.Content->instructions, "Instructions:");
    }

    void operator()(ast::Swap&) const {
        std::cout << indent() << "Swap" << std::endl; 
    }

    void operator()(ast::If& if_) const {
        std::cout << indent() << "If" << std::endl; 

        print_sub(if_.Content->condition, "Condition");
        print_each_sub(if_.Content->instructions, "Instructions");

        for(auto& else_if : if_.Content->elseIfs){
            std::cout << indent() << "ElseIf" << std::endl;

            print_sub(else_if.condition, "Condition");
            print_each_sub(else_if.instructions, "Instructions");
        }

        if(if_.Content->else_){
            std::cout << indent() << "Else" << std::endl;

            print_each_sub((*if_.Content->else_).instructions, "Instructions");
        }
    }

    void operator()(ast::FunctionCall& call) const {
        std::cout << indent() << "FunctionCall " << call.Content->functionName << std::endl; 
        print_each_sub(call.Content->values);
    }

    void operator()(ast::MemberFunctionCall& call) const {
        std::cout << indent() << "Member FunctionCall " << call.Content->object_name << "." << call.Content->function_name << std::endl; 
        print_each_sub(call.Content->values);
    }

    void operator()(ast::BuiltinOperator& builtin) const {
        std::cout << indent() << "Builtin Operator " << (int) builtin.Content->type << std::endl; 
        print_each_sub(builtin.Content->values);
    }

    void operator()(ast::VariableDeclaration& declaration) const {
        std::cout << indent() << "Variable declaration [" << declaration.Content->variableName << "]" << std::endl; 

        if(declaration.Content->value){
            print_sub(*declaration.Content->value);
        }
    }

    void operator()(ast::ArrayDeclaration&) const {
        std::cout << indent() << "Array declaration" << std::endl; 
    }

    void operator()(ast::SuffixOperation& op) const {
        std::cout << indent() << op.Content->variableName << "(suffix)" << (int)op.Content->op << std::endl; 
    }

    void operator()(ast::PrefixOperation& op) const {
        std::cout << indent() << (int)op.Content->op << "(prefix)" << op.Content->variableName << std::endl; 
    }

    void operator()(ast::Assignment& assign) const {
        std::cout << indent() << "Assignment [operator = " << static_cast<int>(assign.Content->op) << " ] " << std::endl;

        print_sub(assign.Content->left_value, "Left Value");
        print_sub(assign.Content->value, "Right Value");
    }

    void operator()(ast::Ternary& ternary) const {
        std::cout << indent() << "Ternary" << std::endl;

        print_sub(ternary.Content->condition, "Condition");
        print_sub(ternary.Content->true_value, "True Value");
        print_sub(ternary.Content->false_value, "False Value");
    }

    void operator()(ast::Return& return_) const {
        print_sub(return_.Content->value, "Function Return");
    }

    void operator()(ast::Litteral& litteral) const {
        std::cout << indent() << "Litteral [" << litteral.value << "]" << std::endl; 
    }

    void operator()(ast::Integer& integer) const {
        std::cout << indent() << "Integer [" << integer.value << "]" << std::endl; 
    }

    void operator()(ast::IntegerSuffix& integer) const {
        std::cout << indent() << "Float (suffix) [" << integer.value << "f]" << std::endl; 
    }

    void operator()(ast::Float& float_) const {
        std::cout << indent() << "Float [" << float_.value << "]" << std::endl; 
    }

    void operator()(ast::True&) const {
        std::cout << indent() << "true" << std::endl; 
    }

    void operator()(ast::Null&) const {
        std::cout << indent() << "null" << std::endl; 
    }

    void operator()(ast::False&) const {
        std::cout << indent() << "false" << std::endl; 
    }

    void operator()(ast::VariableValue& value) const {
        std::cout << indent() << "Variable " << value.Content->var->name();

        for(auto& member : value.Content->memberNames){
            std::cout << "." << member; 
        }

        std::cout << std::endl;
    }

    void operator()(ast::DereferenceValue& value) const {
        std::cout << indent() << "Dereference Variable Value" << std::endl;;

        print_sub(value.Content->ref, "Left Value");
    }

    void operator()(ast::ArrayValue& value) const {
        std::cout << indent() << "Array value [" << value.Content->arrayName << "]" << std::endl; 
        print_sub(value.Content->indexValue);
    }

    void operator()(ast::Expression& value) const {
        std::cout << indent() << "Expression [" << value.Content->operations.size() << "]" << std::endl; 
        ++level;
        visit(*this, value.Content->first);
        for(auto& operation : value.Content->operations){
            std::cout << indent() << (int) operation.get<0>() << std::endl;
            visit(*this, operation.get<1>());
        }
        --level;
    }

    void operator()(ast::Unary& value) const {
        std::cout << indent() << "Unary " << static_cast<int>(value.Content->op) << std::endl; 
        print_sub(value.Content->value);
    }

    void operator()(ast::Cast& cast) const {
        std::cout << indent() << "Cast " << std::endl; 
        std::cout << indent() << "\tType: " << toStringType(cast.Content->type) << std::endl;
        print_sub(cast.Content->value);
    }
};

} //end of anonymous namespace 

void ast::Printer::print(SourceFile& program){
    DebugVisitor visitor;
    visit_non_variant(visitor, program);
}
