///////////////////////////////////////////////////////////////////////////////
//
// This file is subject to the terms and conditions defined in
// file 'LICENSE.txt', which is part of this source code package.
//
///////////////////////////////////////////////////////////////////////////////
// (C) Vilem Otte <vilem.otte@post.cz>
///////////////////////////////////////////////////////////////////////////////
#include "Compiler.h"

// Error function
void Compiler::Expected(const std::string& error)
{
	auto it = mDebugInfo.find(mNextToken - 1);

	std::cout << "Error: " << error << std::endl;
	if (it != mDebugInfo.end())
	{
		std::cout << "At line " << it->second.GetLine() << " in file " << it->second.GetFilename() << std::endl;
	}
	std::cin.get();
	std::exit(-1);
}

// Returns false in case we read whole input, otherwise true
bool Compiler::Look()
{
	if (mNextToken >= mTokens.size())
	{
		return false;
	}
	return true;
}

// Look whether next token is the token we want
bool Compiler::Look(Lexer::Token t)
{
	if (mNextToken >= mTokens.size())
	{
		return false;
	}

	if (mTokens[mNextToken] == t)
	{
		return true;
	}

	return false;
}

// Match current token
void Compiler::Match(Lexer::Token t)
{
	if (mNextToken >= mTokens.size())
	{
		Expected("Unexpected end of file");
	}

	if (mTokens[mNextToken] != t)
	{
		Expected("Unexpected token");
	}

	mNextToken++;
}

// Get value
std::string Compiler::GetValue()
{
	if (mNextToken >= mTokens.size())
	{
		Expected("Unexpected end of file");
	}

	if (mTokens[mNextToken] != Lexer::VALUE)
	{
		Expected("Expected integer value");
	}

	return mData[mNextToken++];
}

// Get value
std::string Compiler::GetIdent()
{
	if (mNextToken >= mTokens.size())
	{
		Expected("Unexpected end of file");
	}

	if (mTokens[mNextToken] != Lexer::IDENT)
	{
		Expected("Expected integer value");
	}

	return mData[mNextToken++];
}

//////////////////////////////////////////////////////////////////////////////
// Integer
// Rule '<integer> ::= [0..9]+'
void Compiler::Integer()
{
	mCodeStack[mCodeStack.size() - 1] << "mov.reg.i32 r0 " << GetValue() << std::endl;
}

//////////////////////////////////////////////////////////////////////////////
// Identifier
// Rule '<ident> ::= [A..z _][A..z 0..1 _]*'
// Param 'declare' specifies whether we declare the identifier or not
void Compiler::Ident(bool declare, bool lvalue)
{
	if (declare)
	{
		mVariables.insert(std::pair<std::string, size_t>(GetIdent(), mStackOffset));
		mCodeStack[mCodeStack.size() - 1] << "push.i32 r0 " << std::endl;
		mStackOffset += 4;
	}
	else
	{
		if (lvalue)
		{
			std::string ident = GetIdent();
			if (mVariables.find(ident) == mVariables.end())
			{
				Expected("Undeclared Identiefier");
			}
			mCodeStack[mCodeStack.size() - 1] << "mov.mem.reg.i32 [sp+" << mVariables[ident] << "] r0 " << std::endl;
		}
		else
		{
			std::string ident = GetIdent();
			if (mVariables.find(ident) == mVariables.end())
			{
				Expected("Undeclared Identiefier");
			}
			mCodeStack[mCodeStack.size() - 1] << "mov.reg.mem.i32 r0 [sp+" << mVariables[ident] << "]" << std::endl;
		}
	}
}

//////////////////////////////////////////////////////////////////////////////
// Factor
// Rule '<factor> ::= (<expr>) | <ident> | <integer>'
void Compiler::Factor()
{
	if (Look(Lexer::LPAREN))
	{
		Match(Lexer::LPAREN);
		Assign();
		Match(Lexer::RPAREN);
	}
	else if (Look(Lexer::IDENT))
	{
		Ident(false, false);
	}
	else
	{
		Integer();
	}
}

//////////////////////////////////////////////////////////////////////////////
// Term 
// Rule '<term> ::= <factor> [<mul_op> <factor>]*'
void Compiler::Term()
{
	Factor();

	while (Look(Lexer::MULTIPLICATION) || Look(Lexer::DIVISION))
	{
		mCodeStack[mCodeStack.size() - 1] << "push.i32 r0" << std::endl;
		if (Look(Lexer::MULTIPLICATION))
		{
			Match(Lexer::MULTIPLICATION);
			Factor();
			mCodeStack[mCodeStack.size() - 1] << "pop.i32 r1" << std::endl;
			mCodeStack[mCodeStack.size() - 1] << "mul.i32 r0 r1" << std::endl;
		}
		else if (Look(Lexer::DIVISION))
		{
			Match(Lexer::DIVISION);
			Factor();
			mCodeStack[mCodeStack.size() - 1] << "pop.i32 r1" << std::endl;
			mCodeStack[mCodeStack.size() - 1] << "div.i32 r1 r0" << std::endl;
			mCodeStack[mCodeStack.size() - 1] << "mov.reg.reg r0 r1" << std::endl;
		}
		else if (Look())
		{
			Expected("Expected addition or subtraction operation");
		}
	}
}

//////////////////////////////////////////////////////////////////////////////
// Sub-Expression (numerical - addition and subtraction ops)
// Rule '<sub> ::= <term> [<add_op> <term>]*'
void Compiler::Sub()
{
	Term();

	while (Look(Lexer::ADDITION) || Look(Lexer::SUBTRACTION))
	{
		mCodeStack[mCodeStack.size() - 1] << "push.i32 r0" << std::endl;
		if (Look(Lexer::ADDITION))
		{
			Match(Lexer::ADDITION);
			Term();
			mCodeStack[mCodeStack.size() - 1] << "pop.i32 r1" << std::endl;
			mCodeStack[mCodeStack.size() - 1] << "add.i32 r0 r1" << std::endl;
		}
		else if (Look(Lexer::SUBTRACTION))
		{
			Match(Lexer::SUBTRACTION);
			Term();
			mCodeStack[mCodeStack.size() - 1] << "pop.i32 r1" << std::endl;
			mCodeStack[mCodeStack.size() - 1] << "sub.i32 r0 r1" << std::endl;
			mCodeStack[mCodeStack.size() - 1] << "neg.i32 r0" << std::endl;
		}
		else if (Look())
		{
			Expected("Expected addition or subtraction operation");
		}
	}
}

//////////////////////////////////////////////////////////////////////////////
// Assignment
// Rule '<assign> ::= <ident> [<assign_op> <ident>]* [<assign_op> <sub>]^ | <sub>'
void Compiler::Assign()
{
	if (!Look(Lexer::IDENT))
	{
		Sub();
	}
	else
	{
		mCodeStack.push_back(std::stringstream());
		Ident(false, true);
		std::string top = mCodeStack[mCodeStack.size() - 1].str();
		mCodeStack.pop_back();

		size_t deep = 0;
		bool variable = true;

		while (Look(Lexer::ASSIGN))
		{
			deep++;
			mCodeStack.push_back(std::stringstream());

			Match(Lexer::ASSIGN);

			if (!Look(Lexer::IDENT))
			{
				variable = false;
				Sub();
			}
			else if (variable)
			{
				Ident(false, true);
			}
			else
			{
				Expected("Left operand must be l-value");
			}
		}

		std::vector<std::string> temp;
		while (deep > 0)
		{
			std::string tmp = mCodeStack[mCodeStack.size() - 1].str();
			temp.insert(temp.begin(), tmp);
			mCodeStack.pop_back();
			deep--;
		}

		for (const std::string& s : temp)
		{
			mCodeStack[mCodeStack.size() - 1] << s;
		}

		mCodeStack[mCodeStack.size() - 1] << top;
	}
}

//////////////////////////////////////////////////////////////////////////////
// Variable declaration
// Rule '<decl> ::= <type><ident> [<assign_op> <assign>]*'
void Compiler::Declaration()
{
	Match(Lexer::TYPE);
	mCodeStack.push_back(std::stringstream());
	Ident(true, true);
	std::string top = mCodeStack[mCodeStack.size() - 1].str();
	mCodeStack.pop_back();

	size_t deep = 0;

	while (Look(Lexer::ASSIGN))
	{
		deep++;
		mCodeStack.push_back(std::stringstream());

		Match(Lexer::ASSIGN);
		Assign();
	}

	std::vector<std::string> temp;
	while (deep > 0)
	{
		std::string tmp = mCodeStack[mCodeStack.size() - 1].str();
		temp.insert(temp.begin(), tmp);
		mCodeStack.pop_back();
		deep--;
	}

	for (const std::string& s : temp)
	{
		mCodeStack[mCodeStack.size() - 1] << s;
	}

	mCodeStack[mCodeStack.size() - 1] << top;
}

//////////////////////////////////////////////////////////////////////////////
// Expression
// Rule '<expr> ::= <decl> | <assign>'
void Compiler::Expression()
{
	// <decl> must begin with <type>
	if (Look(Lexer::TYPE))
	{
		Declaration();
	}
	else
	{
		Assign();
	}
}

//////////////////////////////////////////////////////////////////////////////
// Command
// Rule: <command> ::= <expr><punct>
// Processes single command of the program
void Compiler::Command()
{
	Expression();
	Match(Lexer::PUNCT);
}

// Build program
void Compiler::Program()
{
	while (Look())
	{
		Command();
	}
}

// Construct from lexer, specify output file
Compiler::Compiler(const Lexer& l, const std::string& output)
{
	mTokens = l.GetTokens();
	mData = l.GetData();
	mDebugInfo = l.GetDebugInfo();
	mAssembly.open(output, std::ios::out);
}

// Build
void Compiler::Compile()
{
	mNextToken = 0;
	mCodeStack.push_back(std::stringstream());
	
	Program();

	mAssembly << mCodeStack[mCodeStack.size() - 1].rdbuf();
	mCodeStack.pop_back();
	
	mAssembly.close();
}