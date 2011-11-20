//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include <boost/variant/static_visitor.hpp>

#include "DefaultValues.hpp"

#include "ASTVisitor.hpp"
#include "VisitorUtils.hpp"

#include "Types.hpp"

using namespace eddic;

struct SetDefaultValues : public boost::static_visitor<> {
    AUTO_RECURSE_PROGRAM()
    AUTO_RECURSE_FUNCTION_DECLARATION()
    AUTO_RECURSE_SIMPLE_LOOPS()
    AUTO_RECURSE_FOREACH()
    AUTO_RECURSE_BRANCHES()

    template<typename T>
    void setDefaultValue(T& declaration){
        if(!declaration.Content->value){
            Type type = stringToType(declaration.Content->variableType);

            switch(type.base()){
                case BaseType::INT:{
                    ast::Integer integer;
                    integer.value = 0;

                    declaration.Content->value = integer;

                    break;
                }
                case BaseType::STRING:{
                    ast::Litteral litteral;
                    litteral.value = "\"\"";
                    litteral.label = "S3";

                    declaration.Content->value = litteral;

                    break;
                }
                default:
                    assert(false); //This type is not managed
                    break;
            }
        }
    }

    void operator()(ast::GlobalVariableDeclaration& declaration){
        setDefaultValue(declaration);
    }

    void operator()(ast::VariableDeclaration& declaration){
        setDefaultValue(declaration);
    }

    template<typename T>
    void operator()(T&){
        //No need to recurse further
    }
};

void DefaultValues::fill(ast::Program& program){
    SetDefaultValues visitor;
    visitor(program);
}