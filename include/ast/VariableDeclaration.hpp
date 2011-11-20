//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef AST_VARIABLE_DECLARATION_H
#define AST_VARIABLE_DECLARATION_H

#include <memory>

#include <boost/intrusive_ptr.hpp>
#include <boost/fusion/include/adapt_struct.hpp>

#include "ast/Deferred.hpp"
#include "ast/Value.hpp"

namespace eddic {

class Context;

namespace ast {

struct ASTVariableDeclaration {
    std::shared_ptr<Context> context;

    std::string variableType;
    std::string variableName;
    boost::optional<Value> value;

    mutable long references;
    ASTVariableDeclaration() : references(0) {}
};

typedef Deferred<ASTVariableDeclaration, boost::intrusive_ptr<ASTVariableDeclaration>> VariableDeclaration;

} //end of ast

} //end of eddic

//Adapt the struct for the AST
BOOST_FUSION_ADAPT_STRUCT(
    eddic::ast::VariableDeclaration, 
    (std::string, Content->variableType)
    (std::string, Content->variableName)
    (boost::optional<eddic::ast::Value>, Content->value)
)

#endif