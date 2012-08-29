//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef TEMPLATE_ENGINE_H
#define TEMPLATE_ENGINE_H

#include <memory>
#include <vector>
#include <unordered_map>

#include "ast/source_def.hpp"

namespace eddic {

namespace ast {

class TemplateEngine {
    public:
        typedef std::unordered_multimap<std::string, ast::TemplateFunctionDeclaration> LocalFunctionTemplateMap;
        typedef std::unordered_map<std::string, LocalFunctionTemplateMap> FunctionTemplateMap;

        typedef std::unordered_multimap<std::string, std::vector<std::string>> LocalFunctionInstantiationMap;
        typedef std::unordered_map<std::string, LocalFunctionInstantiationMap> FunctionInstantiationMap;

        void template_instantiation(SourceFile& program);
    
    private:
        FunctionTemplateMap function_templates;
        FunctionInstantiationMap function_template_instantiations;
        
};
    

} //end of ast

} //end of eddic

#endif
