///////////////////////////////////////////////////////////////////////////////
//
// This file is subject to the terms and conditions defined in
// file 'LICENSE.txt', which is part of this source code package.
//
///////////////////////////////////////////////////////////////////////////////
// (C) Vilem Otte <vilem.otte@post.cz>
///////////////////////////////////////////////////////////////////////////////

#include "Disassembler.h"

// Build opcodes database
void Disassembler::BuildOpcodes()
{
	// Pairs string to opcode
	mOpcodes["add.i32"] = ADD_I32;
	mOpcodes["sub.i32"] = SUB_I32;
	mOpcodes["mul.i32"] = MUL_I32;
	mOpcodes["div.i32"] = DIV_I32;
	mOpcodes["push.i32"] = PUSH_I32;
	mOpcodes["pop.i32"] = POP_I32;
	mOpcodes["mov.reg.i32"] = MOV_REG_I32;
	mOpcodes["mov.reg.reg"] = MOV_REG_REG;
	mOpcodes["neg.i32"] = NEG_I32;
	mOpcodes["mov.mem.reg.i32"] = MOV_MEM_REG_I32;
	mOpcodes["mov.reg.mem.i32"] = MOV_REG_MEM_I32;
}

// Get register ID from string
int Disassembler::GetRegister(const std::string& reg)
{
	if (reg == "r0")
	{
		return 0;
	}
	else if (reg == "r1")
	{
		return 1;
	}
	else if (reg == "ip")
	{
		return 2;
	}
	else if (reg == "sp")
	{
		return 3;
	}

	return -1;
}

// Parse address
int Disassembler::ParseAddress(const std::string& token, int& reg, int& offset)
{
	if (token[0] != '[' || token[token.length() - 1] != ']')
	{
		return -1;
	}

	std::string tmp = token.substr(1, token.length() - 2);
	
	size_t sep = tmp.find('+');
	bool add = true;
	if (sep == std::string::npos)
	{
		sep = tmp.find('-');
		add = false;
	}

	std::string r = tmp.substr(0, sep);
	std::string v = tmp.substr(sep + 1);

	reg = GetRegister(r);
	offset = boost::lexical_cast<int>(v);
	
	if (add == false)
	{
		offset = -offset;
	}

	offset += mOffset;

	return 0;
}

// Process assembly line (disassemble single line)
void Disassembler::ProcessLine(const std::string& l)
{
	// Tokenize line
	std::vector<std::string> t;
	std::string lt = l;
	boost::algorithm::trim(lt);
	boost::algorithm::split(t, lt, boost::is_any_of(" "));
	for (std::string& s : t)
	{
		boost::algorithm::trim(s);
	}

	// Write opcode
	int opcode = mOpcodes[t[0]];
	fwrite(&opcode, sizeof(int), 1, mOutput);

	// Write argument(s)
	int temp[3];
	switch (opcode)
	{
	case ADD_I32:
	case SUB_I32:
	case MUL_I32:
	case DIV_I32:
	case MOV_REG_REG:
		temp[0] = GetRegister(t[1]);
		temp[1] = GetRegister(t[2]);
		fwrite(&temp[0], sizeof(int), 1, mOutput);
		fwrite(&temp[1], sizeof(int), 1, mOutput);
		break;

	case PUSH_I32:
		temp[0] = GetRegister(t[1]);
		fwrite(&temp[0], sizeof(int), 1, mOutput);
		mOffset -= 4;
		break;

	case POP_I32:
		temp[0] = GetRegister(t[1]);
		fwrite(&temp[0], sizeof(int), 1, mOutput);
		mOffset += 4;
		break;

	case NEG_I32:
		temp[0] = GetRegister(t[1]);
		fwrite(&temp[0], sizeof(int), 1, mOutput);
		break;

	case MOV_REG_I32:
		temp[0] = GetRegister(t[1]);
		temp[1] = boost::lexical_cast<int>(t[2]);
		fwrite(&temp[0], sizeof(int), 1, mOutput);
		fwrite(&temp[1], sizeof(int), 1, mOutput);
		break;

	case MOV_MEM_REG_I32:
		ParseAddress(t[1], temp[0], temp[1]);
		temp[2] = GetRegister(t[2]);
		fwrite(&temp[0], sizeof(int), 1, mOutput);
		fwrite(&temp[1], sizeof(int), 1, mOutput);
		fwrite(&temp[2], sizeof(int), 1, mOutput);
		break;

	case MOV_REG_MEM_I32:
		temp[0] = GetRegister(t[1]);
		ParseAddress(t[2], temp[1], temp[2]);
		fwrite(&temp[0], sizeof(int), 1, mOutput);
		fwrite(&temp[1], sizeof(int), 1, mOutput);
		fwrite(&temp[2], sizeof(int), 1, mOutput);
		break;
	}
}

// Constructor, pass in binary file and path to output file
Disassembler::Disassembler(const std::string& filename, const std::string& output)
{
	BuildOpcodes();
	mAssembly = Reader::ReadFile(filename);
	fopen_s(&mOutput, output.c_str(), "wb");
}

// Perform disassembly
void Disassembler::Disassemble()
{
	// Line by line disassembly
	for (auto l : mAssembly)
	{
		ProcessLine(l);
	}
	fclose(mOutput);
}