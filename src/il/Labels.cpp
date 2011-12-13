//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include "il/Labels.hpp"

#include "Utils.hpp"

static int currentLabel = 0;

std::string eddic::newLabel(){
   return "L" + toString(currentLabel++); 
}

void eddic::resetNumbering(){
    currentLabel = 0;
}
