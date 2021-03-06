//=======================================================================
// Copyright Baptiste Wicht 2011-2016.
// Distributed under the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#ifndef AST_FUNCTION_DECLARATION_H
#define AST_FUNCTION_DECLARATION_H

#include <string>
#include <memory>
#include <vector>

#include <boost/fusion/include/adapt_struct.hpp>

#include "ast/FunctionParameter.hpp"
#include "ast/Instruction.hpp"
#include "ast/Position.hpp"
#include "ast/VariableType.hpp"

namespace eddic {

class FunctionContext;
class Type;

namespace ast {

/*!
 * \class ASTFunctionDeclaration
 * \brief The AST node for a function declaration.  
 * Should only be used from the Deferred version (eddic::ast::FunctionDeclaration).
 */
struct ASTFunctionDeclaration { 
    std::shared_ptr<FunctionContext> context;
    
    std::string mangledName;
    std::shared_ptr<const eddic::Type> struct_type = nullptr;
    bool standard = false;
    std::string header = "";

    Position position;
    Type returnType;
    std::string functionName;
    std::vector<FunctionParameter> parameters;
    std::vector<Instruction> instructions;

    mutable long references = 0;
};

/*!
 * \typedef FunctionDeclaration
 * \brief The AST node for a function declaration.
 */
typedef Deferred<ASTFunctionDeclaration> FunctionDeclaration; 

} //end of ast

} //end of eddic

//Adapt the struct for the AST
BOOST_FUSION_ADAPT_STRUCT(
    eddic::ast::FunctionDeclaration, 
    (eddic::ast::Position, Content->position)
    (eddic::ast::Type, Content->returnType)
    (std::string, Content->functionName)
    (std::vector<eddic::ast::FunctionParameter>, Content->parameters)
    (std::vector<eddic::ast::Instruction>, Content->instructions)
)

#endif
