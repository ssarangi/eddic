//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef COMPILER_H
#define COMPILER_H

#include <string>

namespace eddic {

/*!
 * \class Compiler
 * \brief The EDDI compiler.
 *
 * This class is used to launch the compilation of a source file. It will then launch each phases of the compilation on this phase
 * and produce either an executable or an assembly file depending on the provided options. 
 */
struct Compiler {
    /*!
     * Compile the given file. 
     * \param file The file to compile. 
     * \return Return code of the compilation process. Numbers other than 0 indicates an error. 
     */
    int compile (const std::string& file);
    
    /*!
     * Compile the given file. The compilation is not timed. 
     * \param file The file to compile. 
     * \return Return code of the compilation process. Numbers other than 0 indicates an error. 
     */
    int compileOnly (const std::string& file);
};

} //end of eddic

#endif
