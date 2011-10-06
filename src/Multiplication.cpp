//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include "Multiplication.hpp"
#include "AssemblyFileWriter.hpp"

using namespace eddic;

void Multiplication::write(AssemblyFileWriter& writer) {
    lhs->write(writer);
    rhs->write(writer);

    writer.stream() << "movl (%esp), %eax" << std::endl;
    writer.stream() << "movl 4(%esp), %ecx" << std::endl;
    writer.stream() << "mull %ecx" << std::endl;
    writer.stream() << "addl $8, %esp" << std::endl;
    writer.stream() << "pushl %eax" << std::endl;
}

int Multiplication::compute(int left, int right) {
    return left * right;
}