//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef SPIRIT_TOKEN_H
#define SPIRIT_TOKEN_H

#include <string>

//TODO Reduce the include to the exact necessary files
#include <boost/spirit/include/classic_position_iterator.hpp>
#include <boost/spirit/include/lex_lexertl.hpp>

namespace eddic {

    typedef std::string::iterator base_iterator_type;
    typedef boost::spirit::classic::position_iterator2<base_iterator_type> pos_iterator_type;
    typedef boost::spirit::lex::lexertl::token<pos_iterator_type, boost::mpl::vector<std::string, int>> Tok;

} //end of eddic

#endif