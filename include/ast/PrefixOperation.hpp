//=======================================================================
// Copyright Baptiste Wicht 2011-2016.
// Distributed under the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#ifndef AST_PREFIX_OPERATION_H
#define AST_PREFIX_OPERATION_H

#include <boost/fusion/include/adapt_struct.hpp>

#include "ast/Deferred.hpp"
#include "ast/Operator.hpp"
#include "ast/Position.hpp"
#include "ast/Value.hpp"

namespace eddic {

namespace ast {

struct ASTPrefixOperation {
    Position position;
    Value left_value;
    ast::Operator op;

    mutable long references = 0;
};

typedef Deferred<ASTPrefixOperation> PrefixOperation;

} //end of ast

} //end of eddic

//Adapt the struct for the AST
BOOST_FUSION_ADAPT_STRUCT(
    eddic::ast::PrefixOperation, 
    (eddic::ast::Position, Content->position)
    (eddic::ast::Operator, Content->op)
    (eddic::ast::Value, Content->left_value)
)

#endif
