//=======================================================================
// Copyright Baptiste Wicht 2011-2016.
// Distributed under the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#ifndef AST_MEMBER_DECLARATION_H
#define AST_MEMBER_DECLARATION_H

#include <boost/fusion/include/adapt_struct.hpp>

#include "ast/Deferred.hpp"
#include "ast/Position.hpp"
#include "ast/VariableType.hpp"

namespace eddic {

class Context;

namespace ast {

/*!
 * \class ASTMemberDeclaration
 * \brief The AST node for a declaration of a member variable. 
 * Should only be used from the Deferred version (eddic::ast::MemberDeclaration).
 */
struct ASTMemberDeclaration {
    Position position;
    Type type;
    std::string name;

    mutable long references = 0;
};

/*!
 * \typedef MemberDeclaration
 * \brief The AST node for a declaration of a member variable. 
 */
typedef Deferred<ASTMemberDeclaration> MemberDeclaration;

} //end of ast

} //end of eddic

//Adapt the struct for the AST
BOOST_FUSION_ADAPT_STRUCT(
    eddic::ast::MemberDeclaration, 
    (eddic::ast::Position, Content->position)
    (eddic::ast::Type, Content->type)
    (std::string, Content->name)
)

#endif
