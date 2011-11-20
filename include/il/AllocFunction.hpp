//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef ALLOC_FUNCTION_H
#define ALLOC_FUNCTION_H

#include "il/Instruction.hpp"

namespace eddic {

class AllocFunction : public Instruction {
    public:
        void write(AssemblyFileWriter& writer);
};

} //end of eddic

#endif