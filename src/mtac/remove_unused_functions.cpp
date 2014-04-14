//=======================================================================
// Copyright Baptiste Wicht 2011-2014.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include "iterators.hpp"
#include "logging.hpp"
#include "GlobalContext.hpp"

#include "mtac/remove_unused_functions.hpp"
#include "mtac/Program.hpp"
#include "mtac/Utils.hpp"
#include "mtac/Quadruple.hpp"

using namespace eddic;

bool mtac::remove_unused_functions::operator()(mtac::Program& program){
    program.call_graph.compute_reachable();

    program.functions.erase(std::remove_if(program.functions.begin(), program.functions.end(), [&program](auto& function){
        if(!program.call_graph.is_reachable(function.definition())){
            LOG<Debug>("Optimizer") << "Remove unused function " << function.get_name() << log::endl;
            return true;
        } 

        return false;
    }), program.functions.end());

    program.call_graph.release_reachable();

    //Not necessary to restart the other passes
    return false;
}
