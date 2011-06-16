//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef LEXER_H
#define LEXER_H

#include <fstream>

#include "CompilerException.h"

enum TokenType {
	NOTHING,
	WORD,
	ASSIGN,
	LITTERAL,
	LEFT_PARENTH,
	RIGHT_PARENTH,
	STOP,
	INTEGER
};

class Lexer {
  private:
    std::ifstream stream;
    std::string currentToken;
    TokenType currentType;
  public:
    void lex(std::string file) throw(CompilerException) ;
    void close();
    bool next();
    std::string getCurrentToken() const;
    bool isWord() const;
    bool isAssign() const;
    bool isLitteral() const;
    bool isParenth() const;
    bool isLeftParenth() const;
    bool isRightParenth() const;
    bool isStop() const;
    bool isInteger() const;
};

#endif
