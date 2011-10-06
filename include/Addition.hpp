//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef ADDITION_H
#define ADDITION_H

#include <memory>

#include "BinaryOperator.hpp"

namespace eddic {

class Addition : public BinaryOperator {
    public:
        Addition(std::shared_ptr<Context> context, const std::shared_ptr<Token> token, std::shared_ptr<Value> lhs, std::shared_ptr<Value> rhs) : BinaryOperator(context, token, lhs, rhs) {}

        void write(AssemblyFileWriter& writer);
        void optimize();

        Type checkTypes(Type left, Type right);
        std::string compute(const std::string& left, const std::string& right);
        int compute(int left, int right);
};

} //end of eddic

#endif