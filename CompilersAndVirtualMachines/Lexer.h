///////////////////////////////////////////////////////////////////////////////
//
// This file is subject to the terms and conditions defined in
// file 'LICENSE.txt', which is part of this source code package.
//
///////////////////////////////////////////////////////////////////////////////
// (C) Vilem Otte <vilem.otte@post.cz>
///////////////////////////////////////////////////////////////////////////////

#ifndef __LEXER_H__
#define __LEXER_H__

#include <string>
#include <vector>
#include <map>
#include <sstream>
#include <fstream>
#include <iostream>
//#include <boost/algorithm/string.hpp>
//#include <boost/lexical_cast.hpp>
//#include <boost/tokenizer.hpp>

#include "Reader.h"
#include "LineInfo.h"

class Lexer
{
public:
	// Tokens we use
	enum Token
	{
		ADDITION = 0,		// +
		SUBTRACTION,		// -
		MULTIPLICATION,		// *
		DIVISION,			// /
		LPAREN,				// (
		RPAREN,				// )
		LEQUAL,				// <=
		GEQUAL,				// >=
		LESS,				// <
		GREATER,			// >
		EQUAL,				// ==
		NOTEQUAL,			// !=
		IF,					// if
		ELSE,				// else
		DO,					// do
		WHILE,				// while
		FOR,				// for
		LBRACE,				// {
		RBRACE,				// }
		IDENT,				// any identifier
		VALUE,				// any value
		ASSIGN,				// = -> assignment operator
		PUNCT,				// ; -> punctuator (semicolon commonly), denotes end of command
		TYPE,				// int -> so far only integers are supported
		DEBUG				// debug info (line number and filename)
	};

private:
	std::vector<std::pair<Token, std::string> > mTokensMap;
	std::vector<std::pair<std::vector<std::string>, Token> > mTokensVars;

	std::vector<std::string> mData;
	std::vector<std::string> mCompilerData;
	std::map<size_t, LineInfo> mDebugInfo;
	std::vector<Token> mTokens;

	void PrepareTokens();

	// Determine whether string is a valid identifier
	bool IsIdent(const std::string& ident);

	// Determine whether string holds a value
	bool IsValue(const std::string& value);

	// Determine whether string holds a type
	bool IsType(const std::string& value);

	bool KeyWord(const std::string& keyword, const std::string& source, size_t pos);

public:
	Lexer(const std::string& filename);

	// Print out file
	void SaveFile(const std::string& filename);

	// Get data
	const std::vector<std::string>& GetData() const
	{
		return mCompilerData;
	}

	// Get tokens
	const std::vector<Token>& GetTokens() const
	{
		return mTokens;
	}

	// Get Debug Info
	const std::map<size_t, LineInfo>& GetDebugInfo() const
	{
		return mDebugInfo;
	}
};

#endif