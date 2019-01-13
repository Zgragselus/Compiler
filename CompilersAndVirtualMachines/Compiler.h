///////////////////////////////////////////////////////////////////////////////
//
// This file is subject to the terms and conditions defined in
// file 'LICENSE.txt', which is part of this source code package.
//
///////////////////////////////////////////////////////////////////////////////
// (C) Vilem Otte <vilem.otte@post.cz>
///////////////////////////////////////////////////////////////////////////////

#ifndef __COMPILER_H__
#define __COMPILER_H__

#include <string>
#include <vector>
#include <map>
#include <sstream>
#include <fstream>
#include <iostream>
#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/tokenizer.hpp>
#include <iostream>
#include "Lexer.h"

/*

* - zero or more repeats
+ - one or more repeats
^ - zero or one

<command> ::= <expr><punct>
<expr> ::= <decl> | <assign>
<decl> ::= <type><ident> [<assign_op> <assign>]*
<assign> ::= <ident> [<assign_op> <ident>]* [<assign_op> <sub>]^ | <sub>
<sub> ::= <term> [<add_op> <term>]*
<term> ::= <factor> [<mul_op> <factor>]*
<factor> ::= (<assign>) | <ident> | <integer>

<type> ::= ["int"]
<ident> ::= [A..z _][A..z 0..1 _]*
<assign_op> ::= [=]
<add_op> ::= [+-]
<mul_op> ::= [/*]
<integer> ::= [0..9]+

*/

class Compiler
{
private:
	std::string mDebug;						// Debug info
	std::vector<Lexer::Token> mTokens;		// Tokens from Lexer
	std::vector<std::string> mData;			// Data from Lexer (string behind tokens)
	std::map<size_t, LineInfo> mDebugInfo;	// Debuginfo map (for each line)
	std::ofstream mAssembly;				// Assembly output stream
	size_t mNextToken;						// Token counter (where we are)

	size_t mStackOffset;						// Stack offset due to variables
	std::map<std::string, size_t> mVariables;	// Maps string names to stack pointer offset
	std::vector<std::stringstream> mCodeStack;	// Allows us to for right-to-left

	// Error function
	void Expected(const std::string& error);

	// Returns false in case we read whole input, otherwise true
	bool Look();

	// Look whether next token is the token we want
	bool Look(Lexer::Token t);

	// Match current token
	void Match(Lexer::Token t);

	// Get value
	std::string GetValue();

	// Get identifier
	std::string GetIdent();

	//////////////////////////////////////////////////////////////////////////////
	// Integer
	// Rule '<integer> ::= [0..9]+'
	void Integer();

	//////////////////////////////////////////////////////////////////////////////
	// Identifier
	// Rule '<ident> ::= [A..z _][A..z 0..1 _]*'
	// Param 'declare' specifies whether we declare the identifier or not
	void Ident(bool declare, bool lvalue);

	//////////////////////////////////////////////////////////////////////////////
	// Factor
	// Rule '<factor> ::= (<expr>) | <ident> | <integer>'
	void Factor();

	//////////////////////////////////////////////////////////////////////////////
	// Term 
	// Rule '<term> ::= <factor> [<mul_op> <factor>]*'
	void Term();

	//////////////////////////////////////////////////////////////////////////////
	// Sub-Expression (numerical - addition and subtraction ops)
	// Rule '<sub> ::= <term> [<add_op> <term>]*'
	void Sub();

	//////////////////////////////////////////////////////////////////////////////
	// Assignment
	// Rule '<assign> ::= <ident> [<assign_op> <ident>]* [<assign_op> <sub>]^ | <sub>'
	void Assign();

	//////////////////////////////////////////////////////////////////////////////
	// Variable declaration
	// Rule '<decl> ::= <type><ident> [<assign_op> <assign>]*'
	void Declaration();

	//////////////////////////////////////////////////////////////////////////////
	// Expression
	// Rule '<expr> ::= <decl><punct> | <assign><punct>'
	void Expression();

	//////////////////////////////////////////////////////////////////////////////
	// Command
	// Rule: <command> ::= <expr><punct>
	// Processes single command of the program
	void Command();

	// Build program
	void Program();

public:
	// Construct from lexer, specify output file
	Compiler(const Lexer& l, const std::string& output);

	// Build
	void Compile();
};

#endif