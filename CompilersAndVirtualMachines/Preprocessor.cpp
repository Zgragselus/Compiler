///////////////////////////////////////////////////////////////////////////////
//
// This file is subject to the terms and conditions defined in
// file 'LICENSE.txt', which is part of this source code package.
//
///////////////////////////////////////////////////////////////////////////////
// (C) Vilem Otte <vilem.otte@post.cz>
///////////////////////////////////////////////////////////////////////////////

#include "Preprocessor.h"

// Remove comments
void Preprocessor::RemoveComments(std::vector<std::pair<LineInfo, std::string> >& lines)
{
	// Multi-line comment state
	bool inMultiLine = false;

	// Loop through all lines
	auto it = lines.begin();
	while (it != lines.end())
	{
		// If we're not in multi-line comment
		if (inMultiLine == false)
		{
			// There is no sense in controlling lines with less than 2 symbols ("/*" or "*/"), continue on next line then
			if ((*it).second.length() < 2)
			{
				it++;
				continue;
			}

			// Helper variable for "/*" and "*/" on single line
			size_t multiLineCommentBegin = 0;
			// Loop character by character
			for (size_t i = 1; i < (*it).second.length(); i++)
			{
				if ((*it).second[i - 1] == '/')
				{
					// If we find "/*", multi-line comment begins
					if ((*it).second[i] == '*' && inMultiLine == false)
					{
						inMultiLine = true;
						multiLineCommentBegin = i - 1;
					}
					// If we find "//" and we're not in multi-line comment, erase the rest of the line
					else if ((*it).second[i] == '/' && inMultiLine == false)
					{
						(*it).second.erase(i - 1, std::string::npos);
						break;
					}
				}
				else if ((*it).second[i - 1] == '*' && inMultiLine == true)
				{
					// If we find "*/" and we're in multi-line comment, erase what is between "/*" and "*/"
					if ((*it).second[i] == '/')
					{
						inMultiLine = false;
						(*it).second.erase(multiLineCommentBegin, i - multiLineCommentBegin);

						// We erased what was in string, we have to restart search
						i = 0;
						continue;
					}
				}
			}

			// Trim the line, if it is empty, erase it
			boost::algorithm::trim((*it).second);
			if ((*it).second.length() == 0)
			{
				it = lines.erase(it);
			}
			else
			{
				it++;
			}
		}
		// When we're in multi line comment
		else
		{
			size_t pos = (*it).second.find("*/");

			// If line is inside multi-line comment, remove line and continue
			if (pos == std::string::npos)
			{
				it = lines.erase(it);
			}
			// Otherwise remove everything from beginning of line to "*/", reset multi-line comment state and continue on this line
			else
			{
				(*it).second.erase(0, pos + 1);
				inMultiLine = false;
			}
		}
	}
}

// Get preprocessed line type
Preprocessor::LineType Preprocessor::GetPreprocessorLineType(const std::string& line)
{
	// Trim the line
	std::string lineTrimmed = line;
	boost::algorithm::trim(lineTrimmed);

	// Check macro tokens (define, ifdef, ifndef, else, elif, endif
	if (boost::algorithm::istarts_with(lineTrimmed, "#define"))
	{
		return LINE_MACRO_DEFINE;
	}
	else if (boost::algorithm::istarts_with(lineTrimmed, "#ifdef"))
	{
		return LINE_MACRO_IFDEF;
	}
	else if (boost::algorithm::istarts_with(lineTrimmed, "#ifndef"))
	{
		return LINE_MACRO_IFNDEF;
	}
	else if (boost::algorithm::istarts_with(lineTrimmed, "#else"))
	{
		return LINE_MACRO_ELSE;
	}
	else if (boost::algorithm::istarts_with(lineTrimmed, "#elif"))
	{
		return LINE_MACRO_ELIF;
	}
	else if (boost::algorithm::istarts_with(lineTrimmed, "#endif"))
	{
		return LINE_MACRO_ENDIF;
	}
	// Check include token
	else if (boost::algorithm::istarts_with(lineTrimmed, "#include"))
	{
		return LINE_INCLUDE;
	}

	// Otherwise it's a code line
	return LINE_CODE;
}

// Get include file name
std::string Preprocessor::GetInclude(const std::string& line)
{
	size_t absBegin = line.find('<');
	size_t absEnd = line.find('>');

	if (absBegin != std::string::npos && absEnd != std::string::npos)
	{
		return line.substr(absBegin + 1, absEnd - absBegin - 1);
	}

	return "";
}

// Copy includes into the file
void Preprocessor::PreprocessIncludes(const std::vector<std::string>& includeDirs, std::vector<std::pair<LineInfo, std::string> >& data)
{
	// Is file in relative folder
	std::string includeName;
	std::string filename;

	// Loop through all lines
	for (auto it = data.begin(); it != data.end(); it++)
	{
		// Get line type
		LineType lType = GetPreprocessorLineType((*it).second);

		switch (lType)
		{
			// If line is #include
		case LINE_INCLUDE:
			includeName = GetInclude((*it).second);

			// If included file name has at least length of 1 character
			if (includeName.length() > 0)
			{
				// Check each include directory whether it contains file
				for (auto itDirs = includeDirs.begin(); itDirs != includeDirs.end(); itDirs++)
				{
					// Create full path+filename
					filename = (*itDirs) + includeName;

					// If file exists
					std::ifstream infile(filename);
					if (infile.good())
					{
						// Read whole included file and insert its contents into this file
						std::vector<std::string> lines = Reader::ReadFile(filename);
						std::vector<std::pair<LineInfo, std::string> > infoLines;
						size_t lineNo = 0;
						for (const std::string& s : lines)
						{
							infoLines.push_back(std::pair<LineInfo, std::string>(LineInfo(filename, lineNo), s));
							lineNo++;
						}
						RemoveComments(infoLines);
						for (auto itTmp = infoLines.begin(); itTmp != infoLines.end(); itTmp++)
						{
							it = data.insert(it, *itTmp);
							it++;
						}
						it = data.erase(it);
					}
				}
			}
			break;

		default:
			break;
		}
	}
}

// Get define on given line
std::string Preprocessor::GetDefine(const std::string& line)
{
	boost::char_separator<char> sep(" ");
	boost::tokenizer< boost::char_separator<char> > tokens(line, sep);
	auto it = tokens.begin();
	it++;
	return *it;
}

// Process ifdef branches
void Preprocessor::ProcessIfdefs(const std::vector<std::string>& defines, std::vector<std::pair<LineInfo, std::string> >& data)
{
	std::vector<std::string> defs;

	// Clear temporary define buffer and add command-line defined defines in it
	defs.clear();
	for (auto it = defines.begin(); it != defines.end(); it++)
	{
		defs.push_back(*it);
	}

	// Helper iterators to help us remove undefined lines
	auto itBeginUndefined = data.end();
	auto itEndUndefined = data.end();
	auto itPrev = data.begin();

	// Are we now in undefined code
	bool undefinedCode = false;
	// Define level
	int defStack = 0;
	// Level where undefined code started
	int undefinedLvl = -1;
	// Are we in undefined state on current line
	bool undefinedState = false;

	// Loop through all lines
	for (auto it = data.begin(); it != data.end();)
	{
		// Get line type
		LineType lType = GetPreprocessorLineType((*it).second);

		switch (lType)
		{
			// Add each define into our define list
		case LINE_MACRO_DEFINE:
			if (undefinedCode == false)
			{
				std::string define = GetDefine((*it).second);
				defs.push_back(define);
				it = data.erase(it);
				continue;
			}
			break;

			// Handle #ifdef
		case LINE_MACRO_IFDEF:
			if (undefinedCode == false)
			{
				std::string define = GetDefine((*it).second);
				if (std::find(defs.begin(), defs.end(), define) == defs.end())
				{
					undefinedCode = true;
					itBeginUndefined = it;
					undefinedLvl = defStack;
				}
				else
				{
					it = data.erase(it);
					continue;
				}
			}
			defStack++;
			break;

			// Handle #ifndef
		case LINE_MACRO_IFNDEF:
			if (undefinedCode == false)
			{
				std::string define = GetDefine((*it).second);
				if (std::find(defs.begin(), defs.end(), define) != defs.end())
				{
					undefinedCode = true;
					itBeginUndefined = it;
					undefinedLvl = defStack;
				}
				else
				{
					it = data.erase(it);
					continue;
				}
			}
			defStack++;
			break;

			// Handle #else
		case LINE_MACRO_ELSE:
			if (undefinedCode == true)
			{
				undefinedCode = false;
				itEndUndefined = it;
				undefinedLvl = -1;
				it = data.erase(itBeginUndefined, itEndUndefined + 1);
				continue;
			}
			else
			{
				undefinedCode = true;
				itBeginUndefined = it;
				undefinedLvl = defStack - 1;
			}
			break;

			// Handle #elif
		case LINE_MACRO_ELIF:
		{
			std::string define = GetDefine((*it).second);
			if (std::find(defs.begin(), defs.end(), define) == defs.end())
			{
				if (undefinedCode == false)
				{
					undefinedCode = true;
					itBeginUndefined = it;
					undefinedLvl = defStack - 1;
				}
			}
			else
			{
				if (undefinedCode == true)
				{
					undefinedCode = false;
					itEndUndefined = it;
					undefinedLvl = -1;
					it = data.erase(itBeginUndefined, itEndUndefined + 1);
					continue;
				}
			}
		}
		break;

		// Handle #endif
		case LINE_MACRO_ENDIF:
			defStack--;
			if (undefinedLvl == defStack && undefinedCode == true)
			{
				undefinedCode = false;
				itEndUndefined = it;
				undefinedLvl = -1;
				it = data.erase(itBeginUndefined, itEndUndefined + 1);
				continue;
			}
			else
			{
				it = data.erase(it);
			}
			break;

		default:
			break;
		}

		itPrev = it;
		it++;
	}
}

// Constructor; input file is passed in as lines (stored in vector); 
// need to specify all subdirectories where headers are searched
// all defines (which are not written in file)
// and filename for generating build info (line number & file)
Preprocessor::Preprocessor(const std::vector<std::string>& input, 
	const std::vector<std::string>& directories, 
	const std::vector<std::string>& defines, 
	const std::string& filename)
{
	// Copy data into preprocessed
	size_t lineNo = 1;
	for (std::string s : input)
	{
		mPreprocessed.push_back(std::pair<LineInfo, std::string>(LineInfo(filename, lineNo), s));
		lineNo++;
	}

	// Remove comments from code
	RemoveComments(mPreprocessed);

	// Preprocess includes 
	PreprocessIncludes(directories, mPreprocessed);

	// Process defines
	ProcessIfdefs(defines, mPreprocessed);
}

// Save preprocessed file to given location
void Preprocessor::Save(const std::string& filename)
{
	std::ofstream f(filename, std::ios::out);
	for (const std::pair<LineInfo, std::string>& s : mPreprocessed)
	{
		LineInfo info = s.first;
		f << info.GetLineInfo() << s.second << std::endl;
	}
	f.close();
}