//=======================================================================
// Copyright Baptiste Wicht 2011-2016.
// Distributed under the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#ifndef AST_RETURN_H
#define AST_RETURN_H

#include <memory>

#include <boost/fusion/include/adapt_struct.hpp>

#include "ast/Deferred.hpp"
#include "ast/Value.hpp"
#include "ast/Position.hpp"

namespace eddic {

class FunctionContext;

namespace ast {

/*!
 * \class ASTReturn
 * \brief The AST node for a return.   
 * Should only be used from the Deferred version (eddic::ast::Return).
 */
struct ASTReturn {
    std::string mangled_name;
    std::shared_ptr<FunctionContext> context;

    Position position;
    Value value;

    mutable long references = 0;
};

/*!
 * \typedef Return
 * \brief The AST node for a return. 
 */
typedef Deferred<ASTReturn> Return;

} //end of ast

} //end of eddic

//Adapt the struct for the AST
BOOST_FUSION_ADAPT_STRUCT(
    eddic::ast::Return, 
    (eddic::ast::Position, Content->position)
    (eddic::ast::Value, Content->value)
)

#endif
