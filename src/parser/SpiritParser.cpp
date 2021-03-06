//=======================================================================
// Copyright Baptiste Wicht 2011-2016.
// Distributed under the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#include <iomanip>
#include <istream>
#include <sstream>
#include <iostream>
#include <string>

#include "boost_cfg.hpp"
#include <boost/config/warning_disable.hpp>
#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/phoenix_core.hpp>
#include <boost/spirit/include/phoenix_operator.hpp>
#include <boost/spirit/include/phoenix_fusion.hpp>
#include <boost/spirit/include/phoenix_stl.hpp>
#include <boost/spirit/include/phoenix_object.hpp>

#include "GlobalContext.hpp"

#include "lexer/SpiritLexer.hpp"

#include "parser/SpiritParser.hpp"
#include "parser/EDDIGrammar.hpp"

namespace qi = boost::spirit::qi;
namespace spirit = boost::spirit;

using namespace eddic;

bool parser::SpiritParser::parse(const std::string& file, ast::SourceFile& program, std::shared_ptr<GlobalContext> context){
    timing_timer timer(context->timing(), "parsing");

    std::ifstream in(file.c_str(), std::ios::binary);
    in.unsetf(std::ios::skipws);

    //Collect the size of the file
    in.seekg(0, std::istream::end);
    std::size_t size(static_cast<size_t>(in.tellg()));
    in.seekg(0, std::istream::beg);

    int current_file = context->new_file(file);

    std::string& file_contents = context->get_file_content(current_file);
    file_contents.resize(size);
    in.read(&file_contents[0], size);

    lexer::pos_iterator_type position_begin(file_contents.begin(), file_contents.end(), file);
    lexer::pos_iterator_type position_end;

    static const lexer::StaticLexer lexer;
    static const parser::EddiGrammar grammar(lexer);

    try {
		bool r = spirit::lex::tokenize_and_parse(
				position_begin, position_end,
				lexer,
				grammar(boost::phoenix::cref(position_begin), current_file),
			   	program);

        if(r && position_begin == position_end) {
            return true;
        } else {
            std::cout << "Parsing failed" << std::endl;

            const auto& pos = position_begin.get_position();
            std::cout <<
                "Error at file " << pos.file << " line " << pos.line << " column " << pos.column << std::endl <<
                "'" << position_begin.get_currentline() << "'" << std::endl <<
                std::setw(pos.column) << " ^- here" << std::endl;

            return false;
        }
    } catch (const qi::expectation_failure<lexer::static_lexer_type::iterator_type>& exception) {
        std::cout << "Parsing failed" << std::endl;

        auto pos_begin = (*exception.first).value().begin();

        const auto& pos = pos_begin.get_position();
        std::cout <<
            "Error at file " << pos.file << " line " << pos.line << " column " << pos.column << " Expecting " << exception.what_ << std::endl <<
            "'" << pos_begin.get_currentline() << "'" << std::endl <<
            std::setw(pos.column) << " ^- here" << std::endl;

        return false;
    }
}
