///////////////////////////////////////////////////////////////////////////////
//
// This file is subject to the terms and conditions defined in
// file 'LICENSE.txt', which is part of this source code package.
//
///////////////////////////////////////////////////////////////////////////////
// (C) Vilem Otte <vilem.otte@post.cz>
///////////////////////////////////////////////////////////////////////////////

#ifndef __PREPROCESSOR_H__
#define __PREPROCESSOR_H__

#include <vector>
#include <string>
#include <fstream>
#include <iostream>
#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/tokenizer.hpp>

#include "Reader.h"
#include "LineInfo.h"

// Preprocessor performs preprocessing (includes, defines, etc.)
// after that it merges all lines into single line (so we can tokenize)
class Preprocessor
{
private:
	// Temporary buffer
	std::vector<std::pair<LineInfo, std::string> > mPreprocessed;

	// Preprocessor line type
	enum LineType
	{
		LINE_CODE,						/// Code line
		LINE_MACRO_DEFINE,				/// #define line
		LINE_MACRO_IFDEF,				/// #ifdef line
		LINE_MACRO_IFNDEF,				/// #ifndef line
		LINE_MACRO_ELSE,				/// #else line
		LINE_MACRO_ELIF,				/// #elif line
		LINE_MACRO_ENDIF,				/// #endif line
		LINE_INCLUDE,					/// #include line
	};

	// Remove comments
	void RemoveComments(std::vector<std::pair<LineInfo, std::string> >& lines);

	// Get preprocessed line type
	LineType GetPreprocessorLineType(const std::string& line);

	// Get include file name
	std::string GetInclude(const std::string& line);

	// Copy includes into the file
	void PreprocessIncludes(const std::vector<std::string>& includeDirs, std::vector<std::pair<LineInfo, std::string> >& data);

	// Get define on given line
	std::string GetDefine(const std::string& line);

	// Process ifdef branches
	void ProcessIfdefs(const std::vector<std::string>& defines, std::vector<std::pair<LineInfo, std::string> >& data);

public:
	// Constructor; input file is passed in as lines (stored in vector); 
	// need to specify all subdirectories where headers are searched
	// all defines (which are not written in file)
	// and filename for generating build info (line number & file)
	Preprocessor(const std::vector<std::string>& input, 
		const std::vector<std::string>& directories, 
		const std::vector<std::string>& defines, 
		const std::string& filename);

	// Save preprocessed file to given location
	void Save(const std::string& filename);
};

#endif