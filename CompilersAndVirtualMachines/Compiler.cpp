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
		// Declared identifier is just pushed on stack
		mVariables.insert(std::pair<std::string, size_t>(GetIdent(), mStackOffset));
		mCodeStack[mCodeStack.size() - 1] << "push.i32 r0 " << std::endl;
		mStackOffset += 4;
	}
	else
	{
		// Either undeclared ident (error) or assignment/read
		if (lvalue)
		{
			// Assignment (e.g. l-value), writing into memory
			std::string ident = GetIdent();
			if (mVariables.find(ident) == mVariables.end())
			{
				Expected("Undeclared Identiefier");
			}
			mCodeStack[mCodeStack.size() - 1] << "mov.mem.reg.i32 [sp+" << mVariables[ident] << "] r0 " << std::endl;
		}
		else
		{
			// Reading from memory
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

void Compiler::MulOp()
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

void Compiler::AddOp()
{
	MulOp();

	while (Look(Lexer::ADDITION) || Look(Lexer::SUBTRACTION))
	{
		mCodeStack[mCodeStack.size() - 1] << "push.i32 r0" << std::endl;
		if (Look(Lexer::ADDITION))
		{
			Match(Lexer::ADDITION);
			MulOp();
			mCodeStack[mCodeStack.size() - 1] << "pop.i32 r1" << std::endl;
			mCodeStack[mCodeStack.size() - 1] << "add.i32 r0 r1" << std::endl;
		}
		else if (Look(Lexer::SUBTRACTION))
		{
			Match(Lexer::SUBTRACTION);
			MulOp();
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

void Compiler::CompareOp()
{
	AddOp();

	while (Look(Lexer::LEQUAL) || Look(Lexer::GEQUAL) || Look(Lexer::LESS) || Look(Lexer::GREATER))
	{
		mCodeStack[mCodeStack.size() - 1] << "push.i32 r0" << std::endl;
		if (Look(Lexer::LEQUAL))
		{
			Match(Lexer::LEQUAL);
			AddOp();
			mCodeStack[mCodeStack.size() - 1] << "pop.i32 r1" << std::endl;
			mCodeStack[mCodeStack.size() - 1] << "cmpleq.i32 r1 r0" << std::endl;
		}
		else if (Look(Lexer::GEQUAL))
		{
			Match(Lexer::GEQUAL);
			AddOp();
			mCodeStack[mCodeStack.size() - 1] << "pop.i32 r1" << std::endl;
			mCodeStack[mCodeStack.size() - 1] << "cmpgeq.i32 r1 r0" << std::endl;
		}
		else if (Look(Lexer::LESS))
		{
			Match(Lexer::LESS);
			AddOp();
			mCodeStack[mCodeStack.size() - 1] << "pop.i32 r1" << std::endl;
			mCodeStack[mCodeStack.size() - 1] << "cmpless.i32 r1 r0" << std::endl;
		}
		else if (Look(Lexer::GREATER))
		{
			Match(Lexer::GREATER);
			AddOp();
			mCodeStack[mCodeStack.size() - 1] << "pop.i32 r1" << std::endl;
			mCodeStack[mCodeStack.size() - 1] << "cmpgreater.i32 r1 r0" << std::endl;
		}
		else if (Look())
		{
			Expected("Expected comparison operation");
		}
	}
}

void Compiler::EqOp()
{
	CompareOp();

	while (Look(Lexer::EQUAL) || Look(Lexer::NOTEQUAL))
	{
		mCodeStack[mCodeStack.size() - 1] << "push.i32 r0" << std::endl;
		if (Look(Lexer::EQUAL))
		{
			Match(Lexer::EQUAL);
			CompareOp();
			mCodeStack[mCodeStack.size() - 1] << "pop.i32 r1" << std::endl;
			mCodeStack[mCodeStack.size() - 1] << "cmpeq.i32 r0 r1" << std::endl;
		}
		else if (Look(Lexer::NOTEQUAL))
		{
			Match(Lexer::NOTEQUAL);
			CompareOp();
			mCodeStack[mCodeStack.size() - 1] << "pop.i32 r1" << std::endl;
			mCodeStack[mCodeStack.size() - 1] << "cmpneq.i32 r0 r1" << std::endl;
		}
		else if (Look())
		{
			Expected("Expected equal or not-equal operation");
		}
	}
}

// Generate new unique label
std::string Compiler::NewLabel()
{
	std::string label = "L" + std::to_string(mLabelCount);
	mLabelCount++;
	return label;
}

// Post label into code
void Compiler::PostLabel(const std::string& label)
{
	mCodeStack[mCodeStack.size() - 1] << label << ":" << std::endl;
}

void Compiler::ControlIf()
{
	std::string labelElse = NewLabel();
	std::string labelEndIf = NewLabel();	

	Match(Lexer::IF);
	Match(Lexer::LPAREN);
	EqOp();
	Match(Lexer::RPAREN);
	mCodeStack[mCodeStack.size() - 1] << "jz " << labelElse << std::endl;
	
	if (Look(Lexer::LBRACE))
	{
		Block();
	}
	else
	{
		Expression();
	}

	if (Look(Lexer::ELSE))
	{
		Match(Lexer::ELSE);
		labelEndIf = NewLabel();
		mCodeStack[mCodeStack.size() - 1] << "jmp " << labelEndIf << std::endl;
		PostLabel(labelElse);

		if (Look(Lexer::LBRACE))
		{
			Block();
		}
		else
		{
			Expression();
		}
	}

	PostLabel(labelEndIf);
}

void Compiler::ControlDo()
{
	std::string labelRepeat = NewLabel();
	std::string labelBreak = NewLabel();
	PostLabel(labelRepeat);

	Match(Lexer::DO);

	if (Look(Lexer::LBRACE))
	{
		Block();
	}
	else
	{
		Expression();
	}

	Match(Lexer::WHILE);
	Match(Lexer::LPAREN);
	Assign();
	Match(Lexer::RPAREN);
	mCodeStack[mCodeStack.size() - 1] << "jz " << labelBreak << std::endl;
	mCodeStack[mCodeStack.size() - 1] << "jnz " << labelRepeat << std::endl;
	PostLabel(labelBreak);
}

void Compiler::ControlWhile()
{
	std::string labelRepeat = NewLabel();
	std::string labelBreak = NewLabel();
	PostLabel(labelRepeat);

	Match(Lexer::WHILE);
	Match(Lexer::LPAREN);
	Assign();
	Match(Lexer::RPAREN);
	mCodeStack[mCodeStack.size() - 1] << "jz " << labelBreak << std::endl;

	if (Look(Lexer::LBRACE))
	{
		Block();
	}
	else
	{
		Expression();
	}

	mCodeStack[mCodeStack.size() - 1] << "jmp " << labelRepeat << std::endl;
	PostLabel(labelBreak);
}

void Compiler::ControlFor()
{
	std::string labelCondition;
}

void Compiler::Control()
{
	if (Look(Lexer::IF))
	{
		ControlIf();
	}
	else if (Look(Lexer::DO))
	{
		ControlDo();
	}
	else if (Look(Lexer::WHILE))
	{
		ControlWhile();
	}
	else if (Look(Lexer::FOR))
	{
		ControlFor();
	}
}

//////////////////////////////////////////////////////////////////////////////
// Assignment
// Rule '<assign> ::= <ident> [<assign_op> <ident>]* [<assign_op> <sub>]^ | <sub>'
void Compiler::Assign()
{
	if (!Look(Lexer::IDENT))
	{
		// If we don't begin with <ident>, 2nd rule takes place
		EqOp();
	}
	else
	{
		// Buffer the assembly output
		mCodeStack.push_back(std::stringstream());
		Ident(false, true);
		std::string top = mCodeStack[mCodeStack.size() - 1].str();
		mCodeStack.pop_back();

		size_t deep = 0;
		bool variable = true;

		// Buffer all the assignments into separate buffer
		while (Look(Lexer::ASSIGN))
		{
			deep++;
			mCodeStack.push_back(std::stringstream());

			Match(Lexer::ASSIGN);

			EqOp();
		}

		// Print out in last in first out way (LIFO)
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
// Rule '<decl> ::= <type><ident> [<assign_op> <assign>]^'
void Compiler::Declaration()
{
	// Buffer assignments
	Match(Lexer::TYPE);
	mCodeStack.push_back(std::stringstream());
	Ident(true, true);
	std::string top = mCodeStack[mCodeStack.size() - 1].str();
	mCodeStack.pop_back();

	// Assignment on the right side is buffered
	size_t deep = 0;
	if (Look(Lexer::ASSIGN))
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

	// Print out in last in first out way (LIFO)
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
		Match(Lexer::PUNCT);
	}
	else if (Look(Lexer::IF) || Look(Lexer::DO) || Look(Lexer::WHILE) || Look(Lexer::FOR))
	{
		Control();
	}
	else
	{
		Assign();
		Match(Lexer::PUNCT);
	}
}

//////////////////////////////////////////////////////////////////////////////
// Command
// Rule: <command> ::= <expr><punct>
// Processes single command of the program
void Compiler::Command()
{
	Expression();
}

// Build block
void Compiler::Block()
{
	Match(Lexer::LBRACE);

	while (!Look(Lexer::RBRACE))
	{
		Command();
	}

	Match(Lexer::RBRACE);
}

// Build program
void Compiler::Program()
{
	// Just loop through commands until end of token stream
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