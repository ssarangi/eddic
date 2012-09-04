//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include "assert.hpp"
#include "Variable.hpp"
#include "Type.hpp"
#include "VisitorUtils.hpp"
#include "FunctionContext.hpp"

#include "mtac/OffsetConstantPropagationProblem.hpp"
#include "mtac/GlobalOptimizations.hpp"
#include "mtac/LiveVariableAnalysisProblem.hpp"

using namespace eddic;

typedef mtac::OffsetConstantPropagationProblem::ProblemDomain ProblemDomain;

ProblemDomain mtac::OffsetConstantPropagationProblem::Boundary(std::shared_ptr<mtac::Function> function){
    pointer_escaped = mtac::escape_analysis(function);

    ProblemDomain::Values values;
    ProblemDomain out(values);
    
    for(auto& variable_pair : function->context->stored_variables()){
        auto variable = variable_pair.second; 

        if(variable->type()->is_array()){
            auto array_size = variable->type()->elements()* variable->type()->data_type()->size() + INT->size();

            if(variable->type()->data_type() == FLOAT){
                for(std::size_t i = INT->size(); i < array_size; i += INT->size()){
                    mtac::Offset offset(variable, i);
                    out[offset] = 0.0;
                }
            } else {
                for(std::size_t i = INT->size(); i < array_size; i += INT->size()){
                    mtac::Offset offset(variable, i);
                    out[offset] = 0;
                }
            }
        } else if(variable->type()->is_custom_type() || variable->type()->is_template()){
            auto struct_size = variable->type()->size();

            for(std::size_t i = 0; i < struct_size; i += INT->size()){
                mtac::Offset offset(variable, i);
                out[offset] = 0;
            }
        }
    }

    return out;
}

ProblemDomain mtac::OffsetConstantPropagationProblem::meet(ProblemDomain& in, ProblemDomain& out){
    auto result = mtac::intersection_meet(in, out);

    //Remove all the temporary
    for(auto it = std::begin(result.values()); it != std::end(result.values());){
        if(auto* ptr = boost::get<std::shared_ptr<Variable>>(&it->second)){
            auto variable = *ptr;

            if (variable->position().isTemporary()){
                it = result.values().erase(it);
            } else {
                ++it;
            }
        } else {
            ++it;
        }
    }

    return result;
}

namespace {

struct ConstantCollector : public boost::static_visitor<> {
    ProblemDomain& out;
    const mtac::Offset& offset;

    ConstantCollector(ProblemDomain& out, const mtac::Offset& offset) : out(out), offset(offset) {}

    void operator()(int value){
        out[offset] = value;
    }
    
    //Warning : Do not pass it by reference to avoid going to the template function
    void operator()(std::string value){
        out[offset] = value;
    }
    
    void operator()(double value){
        out[offset] = value;
    }
    
    void operator()(std::shared_ptr<Variable> variable){
        out[offset] = variable;
    }

    template<typename T>
    void operator()(T&){
        out.erase(offset);
    }
};

} //end of anonymous namespace

ProblemDomain mtac::OffsetConstantPropagationProblem::transfer(std::shared_ptr<mtac::BasicBlock> /*basic_block*/, mtac::Statement& statement, ProblemDomain& in){
    auto out = in;

    if(boost::get<std::shared_ptr<mtac::Quadruple>>(&statement)){
        auto quadruple = boost::get<std::shared_ptr<mtac::Quadruple>>(statement);

        //Store the value assigned to result+arg1
        if(quadruple->op == mtac::Operator::DOT_ASSIGN || quadruple->op == mtac::Operator::DOT_FASSIGN || quadruple->op == mtac::Operator::DOT_PASSIGN){
            if(auto* ptr = boost::get<int>(&*quadruple->arg1)){
                if(!quadruple->result->type()->is_pointer()){
                    mtac::Offset offset(quadruple->result, *ptr);
                    
                    ConstantCollector collector(out, offset);
                    visit(collector, *quadruple->arg2);
                }
            }
        //PDOT Lets escape an offset
        } else if(quadruple->op == mtac::Operator::PDOT){
            if(auto* ptr = boost::get<int>(&*quadruple->arg2)){
                auto variable = boost::get<std::shared_ptr<Variable>>(*quadruple->arg1);
                
                mtac::Offset offset(variable, *ptr);
                escaped.insert(offset);
            }
        }
    }
    //Passing a variable by pointer erases its value
    else if (auto* ptr = boost::get<std::shared_ptr<mtac::Param>>(&statement)){
        auto param = *ptr;

        if(param->address){
            auto variable = boost::get<std::shared_ptr<Variable>>(param->arg);

            //Impossible to know if the variable is modified or not, consider it modified
            for(auto it = std::begin(out.values()); it != std::end(out.values());){
                auto offset = it->first;

                if(offset.variable == variable){
                    it = out.values().erase(it);
                } else {
                    ++it;
                }
            }
        }
    }
            
    //Remove escaped variables from the result
    for(auto it = std::begin(out.values()); it != std::end(out.values());){
        auto offset = it->first;

        if(escaped.find(offset) == escaped.end()){
            ++it;
        } else {
            it = out.values().erase(it);
        }
    }

    return out;
}

bool mtac::OffsetConstantPropagationProblem::optimize(mtac::Statement& statement, std::shared_ptr<mtac::DataFlowResults<ProblemDomain>> global_results){
    auto& results = global_results->IN_S[statement];

    bool changes = false;

    if(boost::get<std::shared_ptr<mtac::Quadruple>>(&statement)){
        auto quadruple = boost::get<std::shared_ptr<mtac::Quadruple>>(statement);

        //If constant replace the value assigned to result by the value stored for arg1+arg2
        if(quadruple->op == mtac::Operator::DOT){
            if(auto* ptr = boost::get<int>(&*quadruple->arg2)){
                mtac::Offset offset(boost::get<std::shared_ptr<Variable>>(*quadruple->arg1), *ptr);

                if(results.find(offset) != results.end() && pointer_escaped->find(offset.variable) == pointer_escaped->end()){
                    quadruple->op = mtac::Operator::ASSIGN;
                    *quadruple->arg1 = results[offset];
                    quadruple->arg2.reset();

                    if(quadruple->result->type()->is_pointer()){
                        if(auto* var_ptr = boost::get<std::shared_ptr<Variable>>(&*quadruple->arg1)){
                            if(!(*var_ptr)->type()->is_pointer()){
                                quadruple->op = mtac::Operator::PASSIGN;
                            }
                        }
                    }
                    

                    changes = true;
                }
            }
        } else if(quadruple->op == mtac::Operator::FDOT){
            if(auto* ptr = boost::get<int>(&*quadruple->arg2)){
                mtac::Offset offset(boost::get<std::shared_ptr<Variable>>(*quadruple->arg1), *ptr);

                if(results.find(offset) != results.end() && pointer_escaped->find(offset.variable) == pointer_escaped->end()){
                    quadruple->op = mtac::Operator::FASSIGN;
                    *quadruple->arg1 = results[offset];
                    quadruple->arg2.reset();

                    changes = true;
                }
            }
        }
    }

    return changes;
}
