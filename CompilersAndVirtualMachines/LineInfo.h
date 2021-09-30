///////////////////////////////////////////////////////////////////////////////
//
// This file is subject to the terms and conditions defined in
// file 'LICENSE.txt', which is part of this source code package.
//
///////////////////////////////////////////////////////////////////////////////
// (C) Vilem Otte <vilem.otte@post.cz>
///////////////////////////////////////////////////////////////////////////////

#ifndef __LINE_INFO__H__
#define __LINE_INFO__H__

#include <string>
//#include <boost/lexical_cast.hpp>

// Struct holding additional lines information
class LineInfo
{
private:
	std::string mFilename;		// File into which given line belongs to
	size_t mLine;				// Original line number

public:
	// Constructor from filename & line number
	LineInfo(const std::string& filename, size_t lineNo)
	{
		mFilename = filename;
		mLine = lineNo;
	}

	// Constructor from string in format "FILENAME|LINE_NUMBER"
	LineInfo(const std::string& line)
	{
		std::string part = line.substr(3, line.length() - 6);
		size_t split = part.find('|');
		std::string num = part.substr(0, split);

		mLine = std::stoi(num);
		mFilename = part.substr(split + 1);
	}

	// Format line info (wrap it with some symbols so we can parse later)
	std::string GetLineInfo()
	{
		std::string result;
		result += "<|>";
		result += std::to_string(mLine);
		result += "|";
		result += mFilename;
		result += "<|>";
		return result;
	}

	const std::string& GetFilename()
	{
		return mFilename;
	}

	size_t GetLine()
	{
		return mLine;
	}
};

#endif