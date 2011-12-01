//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef COMPILER_H
#define COMPILER_H

#include <string>

#include "ast/program_def.hpp"

namespace eddic {

class StringPool;
class FunctionTable;
class IntermediateProgram;

void defineDefaultValues(ast::Program& program);
void defineContexts(ast::Program& program);
void defineVariables(ast::Program& program);
void defineFunctions(ast::Program& program, FunctionTable& table);

void checkTypes(ast::Program& program);
void checkStrings(ast::Program& program, StringPool& pool);

void checkForWarnings(ast::Program& program, FunctionTable& table);

void transform(ast::Program& program);
void optimize(ast::Program& program, FunctionTable& functionTable, StringPool& pool);
void writeIL(ast::Program& program, StringPool& pool, IntermediateProgram& intermediateProgram);
void writeAsm(IntermediateProgram& il, const std::string& file);

void execCommand(const std::string& command);

//TODO Separate brief and following description in doc
/*!
 * \class Compiler
 * \brief The EDDI compiler. 
 * This class is used to launch the compilation of a source file. It will then launch each phases of the compilation on this phase
 * and produce either an executable or an assembly file depending on the provided options. 
 */
struct Compiler {
    int compile (const std::string& file);
};

/*!
 * \brief Produces a warning on the command line. 
 * \param warning The warning message to produce.  
 */
void warn(const std::string& warning);

} //end of eddic

#endif
