//=======================================================================
// Copyright Baptiste Wicht 2011-2016.
// Distributed under the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#include "cpp_utils/assert.hpp"

#include "variant.hpp"
#include "VisitorUtils.hpp"
#include "Variable.hpp"
#include "iterators.hpp"
#include "GlobalContext.hpp"

#include "ast/TransformerEngine.hpp"
#include "ast/SourceFile.hpp"
#include "ast/ASTVisitor.hpp"
#include "ast/GetTypeVisitor.hpp"

using namespace eddic;

namespace {

struct ValueCleaner : public boost::static_visitor<ast::Value> {
    AUTO_RETURN_FALSE(ast::Value)
    AUTO_RETURN_TRUE(ast::Value)
    AUTO_RETURN_NULL(ast::Value)
    AUTO_RETURN_LITERAL(ast::Value)
    AUTO_RETURN_CHAR_LITERAL(ast::Value)
    AUTO_RETURN_FLOAT(ast::Value)
    AUTO_RETURN_INTEGER(ast::Value)
    AUTO_RETURN_INTEGER_SUFFIX(ast::Value)
    AUTO_RETURN_VARIABLE_VALUE(ast::Value)

    ast::Value operator()(ast::Expression& value){
        if(value.Content->operations.empty()){
            return visit(*this, value.Content->first);
        }

        value.Content->first = visit(*this, value.Content->first);

        for(auto& op : value.Content->operations){
            if(ast::has_operation_value(op)){
                if(auto* ptr = boost::get<ast::Value>(&op.get<1>())){
                    op.get<1>() = visit(*this, *ptr);
                } else if(auto* ptr = boost::get<ast::CallOperationValue>(&op.get<1>())){
                    for(auto& v : ptr->values){
                        v = visit(*this, v);
                    }
                }
            }
        }

        assert(value.Content->operations.size() > 0); //Once here, there is no more empty composed value

        return value;
    }

    ast::Value operator()(ast::Cast& cast){
        cast.value = visit(*this, cast.value);

        return cast;
    }

    ast::Value operator()(ast::FunctionCall& functionCall){
        for(auto& value : functionCall.Content->values){
            value = visit(*this, value);
        }

        return functionCall;
    }

    ast::Value operator()(ast::New& new_){
        for(auto& value : new_.values){
            value = visit(*this, value);
        }

        return new_;
    }

    ast::Value operator()(ast::NewArray& new_){
        new_.Content->size = visit(*this, new_.Content->size);

        return new_;
    }

    ast::Value operator()(ast::Assignment& assignment){
        assignment.Content->left_value = visit(*this, assignment.Content->left_value);
        assignment.Content->value = visit(*this, assignment.Content->value);

        return assignment;
    }

    ast::Value operator()(ast::Ternary& ternary){
        ternary.Content->condition = visit(*this, ternary.Content->condition);
        ternary.Content->true_value = visit(*this, ternary.Content->true_value);
        ternary.Content->false_value = visit(*this, ternary.Content->false_value);

        return ternary;
    }

    ast::Value operator()(ast::PrefixOperation& operation){
        operation.Content->left_value = visit(*this, operation.Content->left_value);

        return operation;
    }

    ast::Value operator()(ast::BuiltinOperator& builtin){
        for(auto& value : builtin.Content->values){
            value = visit(*this, value);
        }

        return builtin;
    }
};

struct ValueTransformer : public boost::static_visitor<ast::Value> {
    AUTO_RETURN_FALSE(ast::Value)
    AUTO_RETURN_TRUE(ast::Value)
    AUTO_RETURN_NULL(ast::Value)
    AUTO_RETURN_LITERAL(ast::Value)
    AUTO_RETURN_CHAR_LITERAL(ast::Value)
    AUTO_RETURN_FLOAT(ast::Value)
    AUTO_RETURN_INTEGER(ast::Value)
    AUTO_RETURN_INTEGER_SUFFIX(ast::Value)
    AUTO_RETURN_VARIABLE_VALUE(ast::Value)

    ast::Value operator()(ast::Cast& cast){
        cast.value = visit(*this, cast.value);

        return cast;
    }

    ast::Value operator()(ast::Expression& value){
        value.Content->first = visit(*this, value.Content->first);

        for(auto& op : value.Content->operations){
            if(ast::has_operation_value(op)){
                if(auto* ptr = boost::get<ast::Value>(&op.get<1>())){
                    op.get<1>() = visit(*this, *ptr);
                } else if(auto* ptr = boost::get<ast::CallOperationValue>(&op.get<1>())){
                    for(auto& v : ptr->values){
                        v = visit(*this, v);
                    }
                }
            }
        }

        return value;
    }

    ast::Value operator()(ast::FunctionCall& functionCall){
        for(auto& value : functionCall.Content->values){
            value = visit(*this, value);
        }

        return functionCall;
    }

    ast::Value operator()(ast::New& new_){
        for(auto& value : new_.values){
            value = visit(*this, value);
        }

        return new_;
    }

    ast::Value operator()(ast::NewArray& new_){
        new_.Content->size = visit(*this, new_.Content->size);

        return new_;
    }

    ast::Value operator()(ast::Assignment& assignment){
        assignment.Content->left_value = visit(*this, assignment.Content->left_value);
        assignment.Content->value = visit(*this, assignment.Content->value);

        return assignment;
    }

    ast::Value operator()(ast::Ternary& ternary){
        ternary.Content->condition = visit(*this, ternary.Content->condition);
        ternary.Content->true_value = visit(*this, ternary.Content->true_value);
        ternary.Content->false_value = visit(*this, ternary.Content->false_value);

        return ternary;
    }

    ast::Value operator()(ast::PrefixOperation& operation){
        operation.Content->left_value = visit(*this, operation.Content->left_value);

        return operation;
    }

    ast::Value operator()(ast::BuiltinOperator& builtin){
        for(auto& value : builtin.Content->values){
            value = visit(*this, value);
        }

        return builtin;
    }
};

struct InstructionTransformer : public boost::static_visitor<std::vector<ast::Instruction>> {
    result_type operator()(ast::Assignment& compound) const {
        if(compound.Content->op == ast::Operator::ASSIGN || compound.Content->op == ast::Operator::SWAP){
            return {};
        }

        ast::Expression composed;
        composed.Content->context = compound.Content->context;
        composed.Content->first = compound.Content->left_value;
        composed.Content->operations.push_back(boost::make_tuple(compound.Content->op, compound.Content->value));

        ast::Assignment assignment;
        assignment.Content->context = compound.Content->context;
        assignment.Content->left_value = compound.Content->left_value;
        assignment.Content->value = composed;

        return {assignment};
    }

    //Transform while in do while loop as an optimization (less jumps)
    result_type operator()(ast::While& while_) const {
        ast::If if_;
        if_.Content->context = while_.Content->context;
        if_.Content->condition = while_.Content->condition;

        ast::DoWhile do_while;
        do_while.Content->context = while_.Content->context;
        do_while.Content->condition = while_.Content->condition;
        do_while.Content->instructions = while_.Content->instructions;

        if_.Content->instructions.push_back(do_while);

        return {if_};
    }

    //Transform foreach loop in do while loop
    result_type operator()(ast::Foreach& foreach) const {
        ast::Integer from_value;
        from_value.value = foreach.Content->from;

        ast::Integer to_value;
        to_value.value = foreach.Content->to;

        ast::Expression condition;
        condition.Content->context = foreach.Content->context;
        condition.Content->first = from_value;
        condition.Content->operations.push_back(boost::make_tuple(ast::Operator::LESS_EQUALS, to_value));

        ast::If if_;
        if_.Content->context = foreach.Content->context;
        if_.Content->condition = condition;

        ast::VariableValue left_value;
        left_value.context = foreach.Content->context;
        left_value.variableName = foreach.Content->variableName;
        left_value.var = foreach.Content->context->getVariable(foreach.Content->variableName);

        ast::Assignment start_assign;
        start_assign.Content->context = foreach.Content->context;
        start_assign.Content->left_value = left_value;
        start_assign.Content->value = from_value;

        if_.Content->instructions.push_back(start_assign);

        ast::VariableValue v;
        v.variableName = foreach.Content->variableName;
        v.context = foreach.Content->context;
        v.var = v.context->getVariable(foreach.Content->variableName);

        ast::Expression while_condition;
        while_condition.Content->context = foreach.Content->context;
        while_condition.Content->first = v;
        while_condition.Content->operations.push_back(boost::make_tuple(ast::Operator::LESS_EQUALS, to_value));

        ast::DoWhile do_while;
        do_while.Content->context = foreach.Content->context;
        do_while.Content->condition = while_condition;
        do_while.Content->instructions = foreach.Content->instructions;

        ast::Integer inc;
        inc.value = 1;

        ast::Expression addition;
        addition.Content->context = foreach.Content->context;
        addition.Content->first = v;
        addition.Content->operations.push_back(boost::make_tuple(ast::Operator::ADD, inc));

        ast::Assignment repeat_assign;
        repeat_assign.Content->context = foreach.Content->context;
        repeat_assign.Content->left_value = left_value;
        repeat_assign.Content->value = addition;

        do_while.Content->instructions.push_back(repeat_assign);

        if_.Content->instructions.push_back(do_while);

        return {if_};
    }

    //Transform foreach loop in do while loop
    result_type operator()(ast::ForeachIn& foreach) const {
        result_type instructions;

        auto iterVar = foreach.Content->iterVar;
        auto arrayVar = foreach.Content->arrayVar;
        auto var = foreach.Content->var;

        ast::Integer init_value;
        init_value.value = 0;

        ast::VariableValue left_value;
        left_value.context = foreach.Content->context;
        left_value.variableName = iterVar->name();
        left_value.var = foreach.Content->context->getVariable(iterVar->name());

        ast::Assignment init_assign;
        init_assign.Content->context = foreach.Content->context;
        init_assign.Content->left_value = left_value;
        init_assign.Content->value = init_value;

        instructions.push_back(init_assign);

        ast::VariableValue iter_var_value;
        iter_var_value.var = iterVar;
        iter_var_value.variableName = iterVar->name();
        iter_var_value.context = foreach.Content->context;

        ast::VariableValue array_var_value;
        array_var_value.var = arrayVar;
        array_var_value.variableName = arrayVar->name();
        array_var_value.context = foreach.Content->context;

        ast::BuiltinOperator size_builtin;
        size_builtin.Content->values.push_back(array_var_value);

        if(arrayVar->type()->is_array()){
            size_builtin.Content->type = ast::BuiltinType::SIZE;
        } else {
            size_builtin.Content->type = ast::BuiltinType::LENGTH;
        }

        ast::Expression while_condition;
        while_condition.Content->context = foreach.Content->context;
        while_condition.Content->first = iter_var_value;
        while_condition.Content->operations.push_back(boost::make_tuple(ast::Operator::LESS, size_builtin));

        ast::If if_;
        if_.Content->context = foreach.Content->context;
        if_.Content->condition = while_condition;

        ast::DoWhile do_while;
        do_while.Content->context = foreach.Content->context;
        do_while.Content->condition = while_condition;

        ast::VariableValue array_var_value_2;
        array_var_value_2.var = arrayVar;
        array_var_value_2.variableName = arrayVar->name();
        array_var_value_2.context = foreach.Content->context;

        ast::Expression array_value;
        array_value.Content->context = foreach.Content->context;
        array_value.Content->first = array_var_value_2;
        array_value.Content->operations.push_back(boost::make_tuple(ast::Operator::BRACKET, iter_var_value));

        ast::VariableDeclaration variable_declaration;
        variable_declaration.Content->context = foreach.Content->context;
        variable_declaration.Content->value = array_value;
        variable_declaration.Content->variableName = var->name();

        do_while.Content->instructions.push_back(variable_declaration);

        //Insert all the instructions of the foreach
        std::copy(foreach.Content->instructions.begin(), foreach.Content->instructions.end(), std::back_inserter(do_while.Content->instructions));

        ast::Integer inc;
        inc.value = 1;

        ast::Expression addition;
        addition.Content->context = foreach.Content->context;
        addition.Content->first = iter_var_value;
        addition.Content->operations.push_back(boost::make_tuple(ast::Operator::ADD, inc));

        ast::Assignment repeat_assign;
        repeat_assign.Content->context = foreach.Content->context;
        repeat_assign.Content->left_value = left_value;
        repeat_assign.Content->value = addition;

        do_while.Content->instructions.push_back(repeat_assign);

        if_.Content->instructions.push_back(do_while);

        instructions.push_back(if_);

        return instructions;
    }

    //Transform for loop in do while loop
    result_type operator()(ast::For& for_) const {
        result_type instructions;

        if(for_.Content->start){
            instructions.push_back(*for_.Content->start);
        }

        if(for_.Content->condition){
            ast::DoWhile do_while;
            do_while.Content->context = for_.Content->context;
            do_while.Content->condition = *for_.Content->condition;
            do_while.Content->instructions = for_.Content->instructions;

            if(for_.Content->repeat){
                do_while.Content->instructions.push_back(*for_.Content->repeat);
            }

            ast::If if_;
            if_.Content->context = for_.Content->context;
            if_.Content->condition = *for_.Content->condition;
            if_.Content->instructions.push_back(do_while);

            instructions.push_back(if_);
        } else {
            ast::True condition;

            ast::DoWhile do_while;
            do_while.Content->context = for_.Content->context;
            do_while.Content->condition = condition;
            do_while.Content->instructions = for_.Content->instructions;

            if(for_.Content->repeat){
                do_while.Content->instructions.push_back(*for_.Content->repeat);
            }

            instructions.push_back(do_while);
        }

        return instructions;
    }

    result_type operator()(ast::Switch& switch_){
        auto cases = switch_.Content->cases;
        auto value_type = visit(ast::GetTypeVisitor(), switch_.Content->value);

        if(value_type == INT){
            ast::Expression first_condition;
            first_condition.Content->context = switch_.Content->context;
            first_condition.Content->first = switch_.Content->value;
            first_condition.Content->operations.push_back(boost::make_tuple(ast::Operator::EQUALS, cases[0].value));

            ast::If if_;
            if_.Content->context = switch_.Content->context;
            if_.Content->condition = first_condition;
            if_.Content->instructions = cases[0].instructions;

            for(const auto& case_ : cases){
                ast::Expression condition;
                condition.Content->context = switch_.Content->context;
                condition.Content->first = switch_.Content->value;
                condition.Content->operations.push_back(boost::make_tuple(ast::Operator::EQUALS, case_.value));

                ast::ElseIf else_if;
                else_if.context = case_.context;
                else_if.condition = condition;
                else_if.instructions = case_.instructions;

                if_.Content->elseIfs.push_back(else_if);
            }

            if(switch_.Content->default_case){
                ast::Else else_;
                else_.context = (*switch_.Content->default_case).context;
                else_.instructions = (*switch_.Content->default_case).instructions;

                if_.Content->else_ = else_;
            }

            return {if_};
        } else if(value_type == STRING){
            ast::FunctionCall first_condition;
            first_condition.Content->context = switch_.Content->context;
            first_condition.Content->position = switch_.Content->position;
            first_condition.Content->function_name = "str_equals";
            first_condition.Content->mangled_name = "_F10str_equalsSS";
            first_condition.Content->values.push_back(switch_.Content->value);
            first_condition.Content->values.push_back(cases[0].value);

            ast::If if_;
            if_.Content->context = switch_.Content->context;
            if_.Content->condition = first_condition;
            if_.Content->instructions = cases[0].instructions;

            for(const auto& case_ : cases){
                ast::FunctionCall condition;
                condition.Content->context = switch_.Content->context;
                condition.Content->position = case_.position;
                condition.Content->function_name = "str_equals";
                condition.Content->mangled_name = "_F10str_equalsSS";
                condition.Content->values.push_back(switch_.Content->value);
                condition.Content->values.push_back(case_.value);

                ast::ElseIf else_if;
                else_if.context = case_.context;
                else_if.condition = condition;
                else_if.instructions = case_.instructions;

                if_.Content->elseIfs.push_back(else_if);
            }

            if(switch_.Content->default_case){
                ast::Else else_;
                else_.context = (*switch_.Content->default_case).context;
                else_.instructions = (*switch_.Content->default_case).instructions;

                if_.Content->else_ = else_;
            }

            return {if_};
        } else {
            cpp_unreachable("Unhandled switch value type");
        }
    }

    //No transformation for the other nodes
    template<typename T>
    result_type operator()(T&) const {
        return {};//Empty vector means no transformation
    }
};

struct CleanerVisitor : public boost::static_visitor<> {
    ValueCleaner transformer;

    AUTO_RECURSE_PROGRAM()
    AUTO_RECURSE_ELSE()
    AUTO_RECURSE_FUNCTION_DECLARATION()
    AUTO_RECURSE_TEMPLATE_FUNCTION_DECLARATION()
    AUTO_RECURSE_CONSTRUCTOR()
    AUTO_RECURSE_DESTRUCTOR()
    AUTO_RECURSE_FOREACH()

    AUTO_IGNORE_MEMBER_DECLARATION()
    AUTO_IGNORE_FALSE()
    AUTO_IGNORE_TRUE()
    AUTO_IGNORE_NULL()
    AUTO_IGNORE_LITERAL()
    AUTO_IGNORE_CHAR_LITERAL()
    AUTO_IGNORE_FLOAT()
    AUTO_IGNORE_INTEGER()
    AUTO_IGNORE_INTEGER_SUFFIX()
    AUTO_IGNORE_IMPORT()
    AUTO_IGNORE_STANDARD_IMPORT()

    void operator()(ast::struct_definition& struct_){
        visit_each(*this, struct_.Content->blocks);
    }

    void operator()(ast::If& if_){
        if_.Content->condition = visit(transformer, if_.Content->condition);

        visit_each(*this, if_.Content->instructions);
        visit_each_non_variant(*this, if_.Content->elseIfs);
        visit_optional_non_variant(*this, if_.Content->else_);
    }

    void operator()(ast::ElseIf& elseIf){
        elseIf.condition = visit(transformer, elseIf.condition);

        visit_each(*this, elseIf.instructions);
    }

    void operator()(ast::For& for_){
        visit_optional(*this, for_.Content->start);
        if(for_.Content->condition){
            for_.Content->condition = visit(transformer, *for_.Content->condition);
        }
        visit_optional(*this, for_.Content->repeat);
        visit_each(*this, for_.Content->instructions);
    }

    void operator()(ast::While& while_){
        while_.Content->condition = visit(transformer, while_.Content->condition);
        visit_each(*this, while_.Content->instructions);
    }

    void operator()(ast::DoWhile& while_){
        while_.Content->condition = visit(transformer, while_.Content->condition);
        visit_each(*this, while_.Content->instructions);
    }

    void operator()(ast::Switch& switch_){
        visit_each_non_variant(*this, switch_.Content->cases);
        switch_.Content->value = visit(transformer, switch_.Content->value);
        visit_optional_non_variant(*this, switch_.Content->default_case);
    }

    void operator()(ast::SwitchCase& switch_case){
        switch_case.value = visit(transformer, switch_case.value);
        visit_each(*this, switch_case.instructions);
    }

    void operator()(ast::DefaultCase& default_case){
        visit_each(*this, default_case.instructions);
    }

    void operator()(ast::FunctionCall& functionCall){
        for(auto& value : functionCall.Content->values){
            value = visit(transformer, value);
        }
    }

    void operator()(ast::BuiltinOperator& builtin){
        for(auto& value : builtin.Content->values){
            value = visit(transformer, value);
        }
    }

    void operator()(ast::GlobalArrayDeclaration& declaration){
        declaration.Content->size = visit(transformer, declaration.Content->size);
    }

    void operator()(ast::ArrayDeclaration& declaration){
        declaration.Content->size = visit(transformer, declaration.Content->size);
    }

    void operator()(ast::GlobalVariableDeclaration& declaration){
        if(declaration.Content->value){
            declaration.Content->value = visit(transformer, *declaration.Content->value);
        }
    }

    void operator()(ast::Assignment& assignment){
        assignment.Content->left_value = visit(transformer, assignment.Content->left_value);
        assignment.Content->value = visit(transformer, assignment.Content->value);
    }

    void operator()(ast::Ternary& ternary){
        ternary.Content->condition = visit(transformer, ternary.Content->condition);
        ternary.Content->true_value = visit(transformer, ternary.Content->true_value);
        ternary.Content->false_value = visit(transformer, ternary.Content->false_value);
    }

    void operator()(ast::Return& return_){
        return_.Content->value = visit(transformer, return_.Content->value);
    }

    void operator()(ast::Delete& delete_){
        delete_.Content->value = visit(transformer, delete_.Content->value);
    }

    void operator()(ast::PrefixOperation& operation){
        operation.Content->left_value = visit(transformer, operation.Content->left_value);
    }

    void operator()(ast::StructDeclaration& declaration){
        for(auto& value : declaration.Content->values){
            value = visit(transformer, value);
        }
    }

    void operator()(ast::New& new_){
        for(auto& value : new_.values){
            value = visit(transformer, value);
        }
    }

    void operator()(ast::NewArray& new_){
        new_.Content->size = visit(transformer, new_.Content->size);
    }

    void operator()(ast::VariableDeclaration& declaration){
        if(declaration.Content->value){
            declaration.Content->value = visit(transformer, *declaration.Content->value);
        }
    }

    void operator()(ast::Expression& expression){
        visit_non_variant(transformer, expression);
    }
};

struct TransformerVisitor : public boost::static_visitor<> {
    InstructionTransformer instructionTransformer;
    ValueTransformer transformer;

    AUTO_RECURSE_PROGRAM()

    AUTO_IGNORE_MEMBER_DECLARATION()
    AUTO_IGNORE_TEMPLATE_FUNCTION_DECLARATION()
    AUTO_IGNORE_DELETE()
    AUTO_IGNORE_IMPORT()
    AUTO_IGNORE_STANDARD_IMPORT()
    AUTO_IGNORE_GLOBAL_ARRAY_DECLARATION()
    AUTO_IGNORE_GLOBAL_VARIABLE_DECLARATION()

    void operator()(ast::struct_definition& struct_){
        if(!struct_.Content->is_template_declaration()){
            visit_each(*this, struct_.Content->blocks);
        }
    }

    template<typename T>
    void transform(T& instructions){
        auto start = instructions.begin();
        auto end = instructions.end();

        while(start != end){
            auto transformed = visit(instructionTransformer, *start);

            if(transformed.size() == 1){
                *start = transformed[0];
            } else if(transformed.size() == 2){
                //Replace the current instruction with the first one
                *start = transformed[0];

                //Insert the other instructions after the previously inserted
                start = instructions.insert(start+1, transformed[1]);

                //Update the end iterator
                end = instructions.end();
            }

            ++start;
        }

        visit_each(*this, instructions);
    }

    void operator()(ast::FunctionDeclaration& declaration){
        transform(declaration.Content->instructions);
    }

    void operator()(ast::Constructor& declaration){
        transform(declaration.Content->instructions);
    }

    void operator()(ast::Destructor& declaration){
        transform(declaration.Content->instructions);
    }

    void operator()(ast::If& if_){
        if_.Content->condition = visit(transformer, if_.Content->condition);
        transform(if_.Content->instructions);
        visit_each_non_variant(*this, if_.Content->elseIfs);
        visit_optional_non_variant(*this, if_.Content->else_);
    }

    void operator()(ast::ElseIf& elseIf){
        elseIf.condition = visit(transformer, elseIf.condition);
        transform(elseIf.instructions);
    }

    void operator()(ast::Else& else_){
        transform(else_.instructions);
    }

    void operator()(ast::For& for_){
        visit_optional(*this, for_.Content->start);

        if(for_.Content->condition){
            for_.Content->condition = visit(transformer, *for_.Content->condition);
        }

        visit_optional(*this, for_.Content->repeat);
        transform(for_.Content->instructions);
    }

    void operator()(ast::Foreach& foreach){
        transform(foreach.Content->instructions);
    }

    void operator()(ast::ForeachIn& foreach){
        transform(foreach.Content->instructions);
    }

    void operator()(ast::While& while_){
        while_.Content->condition = visit(transformer, while_.Content->condition);
        transform(while_.Content->instructions);
    }

    void operator()(ast::DoWhile& while_){
        while_.Content->condition = visit(transformer, while_.Content->condition);
        transform(while_.Content->instructions);
    }

    void operator()(ast::FunctionCall& functionCall){
        for(auto& value : functionCall.Content->values){
            value = visit(transformer, value);
        }
    }

    void operator()(ast::Switch& switch_){
        visit_each_non_variant(*this, switch_.Content->cases);
        switch_.Content->value = visit(transformer, switch_.Content->value);
        visit_optional_non_variant(*this, switch_.Content->default_case);
    }

    void operator()(ast::SwitchCase& switch_case){
        switch_case.value = visit(transformer, switch_case.value);
        visit_each(*this, switch_case.instructions);
    }

    void operator()(ast::DefaultCase& default_case){
        visit_each(*this, default_case.instructions);
    }

    void operator()(ast::VariableDeclaration& declaration){
        if(declaration.Content->value){
            declaration.Content->value = visit(transformer, *declaration.Content->value);
        }
    }

    void operator()(ast::Assignment& assignment){
        assignment.Content->left_value = visit(transformer, assignment.Content->left_value);
        assignment.Content->value = visit(transformer, assignment.Content->value);
    }

    void operator()(ast::StructDeclaration& declaration){
        for(auto& value : declaration.Content->values){
            value = visit(transformer, value);
        }
    }

    void operator()(ast::PrefixOperation& operation){
        operation.Content->left_value = visit(transformer, operation.Content->left_value);
    }

    void operator()(ast::Return& return_){
        return_.Content->value = visit(transformer, return_.Content->value);
    }

    void operator()(ast::ArrayDeclaration& declaration){
        declaration.Content->size = visit(transformer, declaration.Content->size);
    }

    void operator()(ast::Expression& expression){
        visit_non_variant(transformer, expression);
    }
};

} //end of anonymous namespace

void ast::CleanPass::apply_program(ast::SourceFile& program, bool){
    CleanerVisitor visitor;
    visitor(program);
}

bool ast::CleanPass::is_simple(){
    return true;
}

bool ast::TransformPass::is_simple(){
    return true;
}

void ast::TransformPass::apply_program(ast::SourceFile& program, bool){
    TransformerVisitor visitor;
    visitor(program);
}
