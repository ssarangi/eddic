//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include <iostream>
#include <iomanip>
#include <fstream>
#include <map>

#include <stdio.h>

#include <commons/ByteCode.h>
#include <commons/Timer.h>

#include "ByteCodeFileWriter.h"
#include "Compiler.h"
#include "Lexer.h"

#include <unistd.h>

using namespace std;

const char* CompilerException::what() throw() {
	return message.c_str();
}

int Compiler::compile(string file){
	cout << "Compile " << file << endl;

	Timer timer;

	ifstream inFile;
	inFile.open(file.c_str());
	inFile.unsetf(ios_base::skipws);

	if(!inFile){
		cout << "Unable to open " << file << endl;
		
		return 1;
	}

	size_t ext_pos = file.find_last_of( '.' );

	string output = "main.v";

    	if ( ext_pos != string::npos ){
		output = file;
		output.replace( ext_pos + 1, output.size() - 1, "v" );
	}

	writer.open(output);
	
	lexer.lex(&inFile);
	
	int code = 0;
	try {
		compile();
	} catch (CompilerException e){
		cout << e.what() << endl;
		code = 1;
	}
	
	inFile.close();
	writer.close();

	if(code != 0){
		remove(output.c_str());
	}
	
	cout << "Compilation took " << timer.elapsed() << "ms" << endl;
	
	return code;
}

void Compiler::compile() throw (CompilerException){
	writer.writeHeader();

	map<string, int> variables;
	int currentVariable = 0;

	while(lexer.next()){
		if(!lexer.isWord()){
			throw CompilerException("An instruction can only start with a call or an assignation");
		}

		string word = lexer.getCurrentToken();

		if(!lexer.next()){
			throw CompilerException("Incomplete instruction");
		}

		if(lexer.isLeftParenth()){ //is a call
			parseCall(word, variables);
		} else if(lexer.isAssign()){ //is an assign
			parseAssign(word, variables, currentVariable);
		} else {
			throw CompilerException("Not an instruction");
		}
	}

	writer.writeEnd();
}

void Compiler::parseCall(string call, map<string, int>& variables) throw (CompilerException){
	if(call != "Print"){
		throw CompilerException("The call \"" + call + "\" does not exist");
	}

	if(!lexer.next()){
		throw CompilerException("Not enough arguments to the call");
	} 

	if(lexer.isLitteral()){
		string litteral = lexer.getCurrentToken();
		writer.writeOneOperandCall(PUSHS, litteral);
	} else if(lexer.isWord()){
		string variable = lexer.getCurrentToken();

		if(variables.find(variable) == variables.end()){
			throw CompilerException("The variable \"" + variable + "\" does not exist");
		}

		writer.writeOneOperandCall(PUSHV, variable);
	} else {
		throw CompilerException("Can only pass litteral or a variable to a call");
	}


	if(!lexer.next() || !lexer.isRightParenth()){
		throw CompilerException("The call must be closed with a right parenth");
	} 

	if(!lexer.next() || !lexer.isStop()){
		throw CompilerException("Every instruction must be closed by a semicolon");
	} 

	writer.writeSimpleCall( PRINT);
} 

void Compiler::parseAssign(string variable, map<string, int>& variables, int& currentVariable) throw (CompilerException){
	if(!lexer.next() || !lexer.isLitteral()){
		throw CompilerException("Need a litteral on the right part of the assignation");
	} 

	string litteral = lexer.getCurrentToken();

	if(!lexer.next() || !lexer.isStop()){
		throw CompilerException("Every instruction must be closed by a semicolon");
	}
	
	if(variables.find(variable) == variables.end()){
		variables[variable] = currentVariable++;
	}
	
	writer.writeOneOperandCall(PUSHS, litteral);
	writer.writeOneOperandCall(ASSIGN, variables[variable]);
}
