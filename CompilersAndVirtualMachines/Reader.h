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

// Reader reads file into buffer
class Reader
{
public:
	// Read file into vector of strings, single record in vector represents single line
	static std::vector<std::string> ReadFile(const std::string& filename);
};

#endif