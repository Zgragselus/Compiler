///////////////////////////////////////////////////////////////////////////////
//
// This file is subject to the terms and conditions defined in
// file 'LICENSE.txt', which is part of this source code package.
//
///////////////////////////////////////////////////////////////////////////////
// (C) Vilem Otte <vilem.otte@post.cz>
///////////////////////////////////////////////////////////////////////////////

#ifndef __READER_H__
#define __READER_H__

#include <string>
#include <vector>
#include <fstream>
#include <iostream>
#include <sstream>

// Reader reads file into buffer
class Reader
{
public:
	// Read file into vector of strings, single record in vector represents single line
	static std::vector<std::string> ReadFile(const std::string& filename);
};

class StringUtil
{
public:
	static std::string& ltrim(std::string& str, const std::string& chars = "\t\n\v\f\r ")
	{
		str.erase(0, str.find_first_not_of(chars));
		return str;
	}

	static std::string& rtrim(std::string& str, const std::string& chars = "\t\n\v\f\r ")
	{
		str.erase(str.find_last_not_of(chars) + 1);
		return str;
	}

	static std::string& trim(std::string & str, const std::string & chars = "\t\n\v\f\r ")
	{
		return ltrim(rtrim(str, chars), chars);
	}

	static std::vector<std::string> split(const std::string& s, char delimiter)
	{
		std::vector<std::string> tokens;
		std::string token;
		std::istringstream tokenStream(s);
		while (std::getline(tokenStream, token, delimiter))
		{
			tokens.push_back(token);
		}
		return tokens;
	}

	static bool starts_with(const std::string& s, const std::string& prefix)
	{
		if (s.rfind(prefix, 0) == 0) 
		{
			return true;
		}

		return false;
	}
};

#endif