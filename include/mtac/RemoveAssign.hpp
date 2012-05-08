//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef MTAC_REMOVE_ASSIGN_H
#define MTAC_REMOVE_ASSIGN_H

#include <memory>
#include <unordered_set>

#include <boost/variant/static_visitor.hpp>

#include "mtac/Quadruple.hpp"
#include "mtac/IfFalse.hpp"
#include "mtac/If.hpp"
#include "mtac/Param.hpp"

#include "mtac/Optimizer.hpp"
#include "mtac/OptimizerUtils.hpp"

namespace eddic {

namespace mtac {

class RemoveAssign : public boost::static_visitor<bool> {
    public:
        bool optimized;
        Pass pass;

        RemoveAssign() : optimized(false) {}

        bool operator()(std::shared_ptr<mtac::Quadruple>& quadruple);
        bool operator()(std::shared_ptr<mtac::Param>& param);
        bool operator()(std::shared_ptr<mtac::IfFalse>& ifFalse);
        bool operator()(std::shared_ptr<mtac::If>& if_);

        template<typename T>
        bool operator()(T&) const { 
            return true;
        }
    
        void collect(mtac::Argument* arg);
        void collect_optional(boost::optional<mtac::Argument>& arg);

    private:
        std::unordered_set<std::shared_ptr<Variable>> used;
};


} //end of mtac

} //end of eddic

#endif