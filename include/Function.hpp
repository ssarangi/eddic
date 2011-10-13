//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef FUNCTION_H
#define FUNCTION_H

#include <vector>
#include <string>
#include <memory>

#include "ParseNode.hpp"
#include "Types.hpp"

namespace eddic {

class Parameter;

class Function : public ParseNode {
	private:
		std::string m_name;
        std::vector<std::shared_ptr<Parameter>> m_parameters;
        int m_currentPosition;
        
        mutable std::string m_mangled_name;

	public:
		Function(std::shared_ptr<Context> context, const Tok token, const std::string& name);
		
        void write(AssemblyFileWriter& writer);

        const std::string& name() const;
        const std::string& mangledName() const;

        void addParameter(const std::string& name, Type type);
};

} //end of eddic

#endif
