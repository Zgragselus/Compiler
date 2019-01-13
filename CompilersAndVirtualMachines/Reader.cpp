///////////////////////////////////////////////////////////////////////////////
//
// This file is subject to the terms and conditions defined in
// file 'LICENSE.txt', which is part of this source code package.
//
///////////////////////////////////////////////////////////////////////////////
// (C) Vilem Otte <vilem.otte@post.cz>
///////////////////////////////////////////////////////////////////////////////

#include "Reader.h"

// Read file into vector of strings, single record in vector represents single line
std::vector<std::string> Reader::ReadFile(const std::string& filename)
{
	std::vector<std::string> result;
	std::ifstream f(filename, std::ios::in);
	std::string line;
	while (std::getline(f, line, '\n'))
	{
		result.push_back(line);
	}
	f.close();
	return result;
}