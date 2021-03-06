//=======================================================================
// Copyright Baptiste Wicht 2011-2016.
// Distributed under the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#ifndef MTAC_PASS_TRAITS_H
#define MTAC_PASS_TRAITS_H

#include <boost/utility.hpp>

#define STATIC_CONSTANT(type,name,value) BOOST_STATIC_CONSTANT(type, name = value)
#define STATIC_STRING(name,value) static inline constexpr const char* name(){ return value; }

namespace eddic {

namespace mtac {

enum class pass_type : unsigned int {
    IPA,
    IPA_SUB,
    LOCAL, 
    DATA_FLOW,
    BB,
    BB_TWO_PASS,
    CUSTOM
};

enum TODO {
    TODO_NONE = 1 //Just here to not let the enum empty
};

enum PROPERTY {
    PROPERTY_POOL = 1,
    PROPERTY_PLATFORM = 2,
    PROPERTY_CONFIGURATION = 4,
    PROPERTY_PROGRAM = 8
};

template<typename T>
struct pass_traits {

};

} //end of mtac

} //end of eddic

#endif
