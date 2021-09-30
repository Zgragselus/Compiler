///////////////////////////////////////////////////////////////////////////////
//
// This file is subject to the terms and conditions defined in
// file 'LICENSE.txt', which is part of this source code package.
//
///////////////////////////////////////////////////////////////////////////////
// (C) Vilem Otte <vilem.otte@post.cz>
///////////////////////////////////////////////////////////////////////////////

#ifndef __DISASSEMBLER_H__
#define __DISASSEMBLER_H__

#include "Reader.h"
#include <map>
//#include <boost/algorithm/string.hpp>
//#include <boost/lexical_cast.hpp>
//#include <boost/tokenizer.hpp>

class Disassembler
{
public:
	// Supported instructions
	enum Instructions
	{
		ADD_I32,			// Add 2 int registers together (result in r0)
		SUB_I32,			// Subtract 2 int registers (result in r0)
		MUL_I32,			// Multiply 2 int registers (result in r0)
		DIV_I32,			// Divide 2 int registers  (result in r0)
		PUSH_I32,			// Push register value onto stack
		POP_I32,			// Pop register value from stack
		MOV_REG_I32,		// Move value (constant) into register
		MOV_REG_REG,		// Move register value into another register
		NEG_I32,			// Negate value in register
		MOV_MEM_REG_I32,	// Move data from register into memory
		MOV_REG_MEM_I32,	// Move data from memory into register
		CMPLEQ_I32,
		CMPGEQ_I32,
		CMPLESS_I32,
		CMPGREATER_I32,
		CMPEQ_I32,
		CMPNEQ_I32,
		JMP,
		JZ,
		JNZ
	};

private:
	FILE* mOutput;							// Disassembled output
	std::vector<std::string> mAssembly;		// Assembly input

	std::map<std::string, int> mOpcodes;	// Opcodes database

	std::map<std::string, int> mLabels;
	int mLabelsCount;
	std::map<int, int> mLabelOffset;

	size_t mOffset;							

	std::string mOutputFilename;

	// Build opcodes database
	void BuildOpcodes();

	// Get register ID from string
	int GetRegister(const std::string& reg);

	// Parse address
	int ParseAddress(const std::string& token, int& reg, int& offset);

	// Process assembly line (disassemble single line)
	void ProcessLine(const std::string& l);

	int GetLabel(const std::string& name);

	int GetLabelOffset(int labelID);

	void StoreLabel(const std::string& name, int position);

	void ResolveLabels();

public:
	// Constructor, pass in binary file and path to output file
	Disassembler(const std::string& filename, const std::string& output);

	// Perform disassembly
	void Disassemble();
};

#endif