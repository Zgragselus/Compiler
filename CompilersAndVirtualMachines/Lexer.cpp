///////////////////////////////////////////////////////////////////////////////
//
// This file is subject to the terms and conditions defined in
// file 'LICENSE.txt', which is part of this source code package.
//
///////////////////////////////////////////////////////////////////////////////
// (C) Vilem Otte <vilem.otte@post.cz>
///////////////////////////////////////////////////////////////////////////////

#include "Lexer.h"

void Lexer::PrepareTokens()
{
	mTokensMap.push_back(std::pair<Token, std::string>(ADDITION, "<add>"));
	mTokensMap.push_back(std::pair<Token, std::string>(SUBTRACTION, "<sub>"));
	mTokensMap.push_back(std::pair<Token, std::string>(MULTIPLICATION, "<mul>"));
	mTokensMap.push_back(std::pair<Token, std::string>(DIVISION, "<div>"));
	mTokensMap.push_back(std::pair<Token, std::string>(LPAREN, "<paren_l>"));
	mTokensMap.push_back(std::pair<Token, std::string>(RPAREN, "<paren_r>"));
	mTokensMap.push_back(std::pair<Token, std::string>(LEQUAL, "<lequal>"));
	mTokensMap.push_back(std::pair<Token, std::string>(GEQUAL, "<gequal>"));
	mTokensMap.push_back(std::pair<Token, std::string>(LESS, "<less>"));
	mTokensMap.push_back(std::pair<Token, std::string>(GREATER, "<greater>"));
	mTokensMap.push_back(std::pair<Token, std::string>(EQUAL, "<equal>"));
	mTokensMap.push_back(std::pair<Token, std::string>(NOTEQUAL, "<notequal>"));
	mTokensMap.push_back(std::pair<Token, std::string>(IF, "<if>"));
	mTokensMap.push_back(std::pair<Token, std::string>(ELSE, "<else>"));
	mTokensMap.push_back(std::pair<Token, std::string>(DO, "<do>"));
	mTokensMap.push_back(std::pair<Token, std::string>(WHILE, "<while>"));
	mTokensMap.push_back(std::pair<Token, std::string>(FOR, "<for>"));
	mTokensMap.push_back(std::pair<Token, std::string>(LBRACE, "<brace_l>"));
	mTokensMap.push_back(std::pair<Token, std::string>(RBRACE, "<brace_r>"));
	mTokensMap.push_back(std::pair<Token, std::string>(IDENT, "<ident>"));
	mTokensMap.push_back(std::pair<Token, std::string>(VALUE, "<value>"));
	mTokensMap.push_back(std::pair<Token, std::string>(ASSIGN, "<assign>"));
	mTokensMap.push_back(std::pair<Token, std::string>(PUNCT, "<punct>"));
	mTokensMap.push_back(std::pair<Token, std::string>(TYPE, "<type>"));
	mTokensMap.push_back(std::pair<Token, std::string>(DEBUG, "<debug>"));

	mTokensVars.push_back(std::pair<std::vector<std::string>, Token>({ "+" }, ADDITION));
	mTokensVars.push_back(std::pair<std::vector<std::string>, Token>({ "-" }, SUBTRACTION));
	mTokensVars.push_back(std::pair<std::vector<std::string>, Token>({ "*" }, MULTIPLICATION));
	mTokensVars.push_back(std::pair<std::vector<std::string>, Token>({ "/" }, DIVISION));
	mTokensVars.push_back(std::pair<std::vector<std::string>, Token>({ "(" }, LPAREN));
	mTokensVars.push_back(std::pair<std::vector<std::string>, Token>({ ")" }, RPAREN));
	mTokensVars.push_back(std::pair<std::vector<std::string>, Token>({ "<=" }, LEQUAL));
	mTokensVars.push_back(std::pair<std::vector<std::string>, Token>({ ">=" }, GEQUAL));
	mTokensVars.push_back(std::pair<std::vector<std::string>, Token>({ "<" }, LESS));
	mTokensVars.push_back(std::pair<std::vector<std::string>, Token>({ ">" }, GREATER));
	mTokensVars.push_back(std::pair<std::vector<std::string>, Token>({ "==" }, EQUAL));
	mTokensVars.push_back(std::pair<std::vector<std::string>, Token>({ "!=" }, NOTEQUAL));
	mTokensVars.push_back(std::pair<std::vector<std::string>, Token>({ "if" }, IF));
	mTokensVars.push_back(std::pair<std::vector<std::string>, Token>({ "else" }, ELSE));
	mTokensVars.push_back(std::pair<std::vector<std::string>, Token>({ "do" }, DO));
	mTokensVars.push_back(std::pair<std::vector<std::string>, Token>({ "while" }, WHILE));
	mTokensVars.push_back(std::pair<std::vector<std::string>, Token>({ "for" }, FOR));
	mTokensVars.push_back(std::pair<std::vector<std::string>, Token>({ "{" }, LBRACE));
	mTokensVars.push_back(std::pair<std::vector<std::string>, Token>({ "}" }, RBRACE));
	mTokensVars.push_back(std::pair<std::vector<std::string>, Token>({}, IDENT));
	mTokensVars.push_back(std::pair<std::vector<std::string>, Token>({}, VALUE));
	mTokensVars.push_back(std::pair<std::vector<std::string>, Token>({ "=" }, ASSIGN));
	mTokensVars.push_back(std::pair<std::vector<std::string>, Token>({ ";" }, PUNCT));
	mTokensVars.push_back(std::pair<std::vector<std::string>, Token>({}, TYPE));
	mTokensVars.push_back(std::pair<std::vector<std::string>, Token>({}, DEBUG));
}

// Determine whether string is a valid identifier
bool Lexer::IsIdent(const std::string& ident)
{
	if (ident.length() < 1)
	{
		return false;
	}

	// Identified always begins with alpha value or underscore
	if (isalpha(ident[0]) || ident[0] == '_')
	{
		// After first symbol, also numbers are allowed (e.g. alnum or underscore)
		for (size_t i = 1; i < ident.length(); i++)
		{
			if (isalnum(ident[i]) || ident[i] == '_')
			{
				continue;
			}
			else
			{
				return false;
			}
		}

		return true;
	}
	else
	{
		return false;
	}
}

// Determine whether string holds a value
bool Lexer::IsValue(const std::string& value)
{
	// Test char
	if (value[0] == '\'' && value[2] == '\'' && value.length() == 3)
	{
		return true;
	}

	// Test string
	if (value[0] == '"' && value[value.length() - 1] == '"')
	{
		for (size_t i = 1; i < value.length() - 1; i++)
		{
			if (value[i] == '"')
			{
				return false;
			}
		}
		return true;
	}

	// Test floating point
	size_t tmp = value.find('.');
	if (tmp != std::string::npos)
	{
		if (value[0] == '.')
		{
			return false;
		}

		if (value[value.length() - 1] != 'f')
		{
			return false;
		}

		for (size_t i = 0; i < value.length() - 1; i++)
		{
			if (i == tmp)
			{
				continue;
			}

			if (!isdigit(value[i]))
			{
				return false;
			}
		}

		return true;
	}

	// Test integer
	for (size_t i = 0; i < value.length(); i++)
	{
		if (!isdigit(value[i]))
		{
			return false;
		}
	}
	return true;
}

bool Lexer::IsType(const std::string& value)
{
	if (value == "int")
	{
		return true;
	}

	return false;
}

bool Lexer::KeyWord(const std::string& keyword, const std::string& source, size_t pos)
{
	if (source.substr(pos, 3) == "int" &&
		(pos == 0 || source[pos - 1] == '#' || source[pos - 1] == ' ' || source[pos - 1] == ';') &&
		(pos == source.length() - 3 || source[pos + 3] == '#' || source[pos + 3] == ' ' || source[pos + 3] == ';'))
	{
		return true;
	}

	return false;
}

Lexer::Lexer(const std::string& filename)
{
	PrepareTokens();

	// Get maximum length token
	size_t MAX_LENGTH_TOKEN = 0;
	for (auto j : mTokensVars)
	{
		for (auto k : j.first)
		{
			if (k.length() > MAX_LENGTH_TOKEN)
			{
				MAX_LENGTH_TOKEN = k.length();
			}
		}
	}

	std::ifstream f(filename, std::ios::in);
	std::stringstream strStream;
	strStream << f.rdbuf();
	std::string joined = strStream.str();
	f.close();

	bool lineInfo = false;
	bool match = true;
	for (size_t i = 0; i < joined.length(); i++)
	{
	joined_loop_restart:
		if (i < joined.length() - 3)
		{
			if (joined[i] == '<' && joined[i + 1] == '|' && joined[i + 2] == '>')
			{
				if (lineInfo == true)
				{
					joined.insert(i + 3, "#");
				}
				else
				{
					joined.insert(i, "#");
				}

				lineInfo = !lineInfo;
				i += 3;
				continue;
			}
		}

		if (lineInfo == true)
		{
			continue;
		}

		if (joined[i] == '\n')
		{
			joined[i] = ' ';
		}
		else if (joined[i] == '\'')
		{
			match = !match;
		}
		else if (joined[i] == '\"')
		{
			match = !match;
		}

		if (match == false)
		{
			continue;
		}

		std::string tmp = joined.substr(i, MAX_LENGTH_TOKEN);
		for (auto j : mTokensVars)
		{
			for (auto k : j.first)
			{
				if (StringUtil::starts_with(tmp, k))
				{
					joined.insert(i, "#");
					joined.insert(i + k.length() + 1, "#");
					i += (k.length() + 1);
					goto joined_loop_restart;
				}
			}
		}

		// Matching keywords
		if (KeyWord("int", joined, i))
		{
			joined.insert(i, "#");
			joined.insert(i + 4, "#");
			i += 4;
		}
		else if (KeyWord("if", joined, i))
		{
			joined.insert(i, "#");
			joined.insert(i + 3, "#");
			i += 3;
		}
		else if (KeyWord("else", joined, i))
		{
			joined.insert(i, "#");
			joined.insert(i + 5, "#");
			i += 5;
		}
		else if (KeyWord("do", joined, i))
		{
			joined.insert(i, "#");
			joined.insert(i + 3, "#");
			i += 3;
		}
		else if (KeyWord("while", joined, i))
		{
			joined.insert(i, "#");
			joined.insert(i + 6, "#");
			i += 6;
		}
		else if (KeyWord("for", joined, i))
		{
			joined.insert(i, "#");
			joined.insert(i + 4, "#");
			i += 4;
		}
	}

	mData = StringUtil::split(joined, '#');
	for (std::vector<std::string>::iterator it = mData.begin(); it != mData.end(); it++)
	{
		StringUtil::trim(*it);
		if ((*it).length() == 0)
		{
			it = mData.erase(it);
			if (it == mData.end())
			{
				break;
			}
		}
	}

	size_t current = 0;
	LineInfo debugInfo = LineInfo("", 0);
	for (size_t i = 0; i < mData.size(); i++)
	{
		if (StringUtil::starts_with(mData[i], "<|>"))
		{
			debugInfo = LineInfo(mData[i]);
			continue;
		}

		bool ident = true;
		const std::string& token = mData[i];
		for (auto j : mTokensVars)
		{
			for (auto k : j.first)
			{
				if (k == token)
				{
					mCompilerData.push_back(token);
					mTokens.push_back(j.second);
					mDebugInfo.insert(std::pair<size_t, LineInfo>(current, debugInfo));
					current++;
					ident = false;
					break;
				}
			}

			if (ident == false)
			{
				break;
			}
		}

		if (ident == true)
		{
			if (IsType(token))
			{
				mCompilerData.push_back(token);
				mTokens.push_back(TYPE);
				mDebugInfo.insert(std::pair<size_t, LineInfo>(current, debugInfo));
				current++;
			}
			else if (IsValue(token))
			{
				mCompilerData.push_back(token);
				mTokens.push_back(VALUE);
				mDebugInfo.insert(std::pair<size_t, LineInfo>(current, debugInfo));
				current++;
			}
			else if (IsIdent(token))
			{
				mCompilerData.push_back(token);
				mTokens.push_back(IDENT);
				mDebugInfo.insert(std::pair<size_t, LineInfo>(current, debugInfo));
				current++;
			}
			else
			{
				std::cout << "Error: Invalid token at " << token << std::endl;
				std::exit(-1);
			}
		}
	}
}

// Print out file
void Lexer::SaveFile(const std::string& filename)
{
	std::ofstream f(filename, std::ios::out);
	for (size_t i = 0; i < mTokens.size(); i++)
	{
		if (mTokens[i] == IDENT)
		{
			f << mCompilerData[i] << " ";
		}
		else if (mTokens[i] == VALUE)
		{
			f << mCompilerData[i] << " ";
		}
		else
		{
			size_t j = 0;
			for (j = 0; j < mTokensMap.size(); j++)
			{
				if (mTokensMap[j].first == mTokens[i])
				{
					break;
				}
			}
			f << mTokensMap[j].second << " ";
		}
	}
	f.close();
}