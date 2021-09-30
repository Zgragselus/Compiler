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
	mOpcodes["cmpleq.i32"] = CMPLEQ_I32;
	mOpcodes["cmpgeq.i32"] = CMPGEQ_I32;
	mOpcodes["cmpless.i32"] = CMPLESS_I32;
	mOpcodes["cmpgreater.i32"] = CMPGREATER_I32;
	mOpcodes["cmpeq.i32"] = CMPEQ_I32;
	mOpcodes["cmpneq.i32"] = CMPNEQ_I32;
	mOpcodes["jmp"] = JMP;
	mOpcodes["jz"] = JZ;
	mOpcodes["jnz"] = JNZ;
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
	offset = std::stoi(v);
	
	if (add == false)
	{
		offset = -offset;
	}

	offset += mOffset;

	return 0;
}

int Disassembler::GetLabel(const std::string& name)
{
	std::cout << "GET LABEL " << name;

	auto it = mLabels.find(name);
	if (it == mLabels.end())
	{
		int labelId = mLabelsCount++;

		mLabels.insert(std::pair<std::string, int>(name, labelId));
		mLabelOffset.insert(std::pair<int, int>(labelId, -1));

		std::cout << " resolved to (" << labelId << ")" << std::endl;

		return labelId;
	}
	else
	{
		std::cout << " resolved to (" << mLabels[name] << ")" << std::endl;

		return mLabels[name];
	}
}

int Disassembler::GetLabelOffset(int labelID)
{
	std::cout << "\tLABEL (" << labelID << ") at " << (mLabelOffset[labelID] / 4) << std::endl;
	return mLabelOffset[labelID];
}

void Disassembler::StoreLabel(const std::string& name, int position)
{
	int labelId = GetLabel(name);
	mLabelOffset[labelId] = position;

	std::cout << "LABEL (" << labelId << ") " << name << " at " << (position / 4) << std::endl;
}

void Disassembler::ResolveLabels()
{
	rewind(mOutput);

	// Line by line disassembly
	for (auto l : mAssembly)
	{
		// Tokenize line
		std::string lt = l;
		StringUtil::trim(lt);
		std::vector<std::string> t = StringUtil::split(lt, ' ');
		for (std::string& s : t)
		{
			StringUtil::trim(s);
		}

		// Write opcode
		int opcode = mOpcodes[t[0]];
		fseek(mOutput, sizeof(int) * 1, SEEK_CUR);

		// Write argument(s)
		int temp[2] = { 0, 0 };
		size_t offset = 0;
		switch (opcode)
		{
		case ADD_I32:
		case SUB_I32:
		case MUL_I32:
		case DIV_I32:
		case MOV_REG_REG:
		case CMPLEQ_I32:
		case CMPGEQ_I32:
		case CMPLESS_I32:
		case CMPGREATER_I32:
		case CMPEQ_I32:
		case CMPNEQ_I32:
			fseek(mOutput, sizeof(int) * 2, SEEK_CUR);
			break;

		case PUSH_I32:
			fseek(mOutput, sizeof(int) * 1, SEEK_CUR);
			break;

		case POP_I32:
			fseek(mOutput, sizeof(int) * 1, SEEK_CUR);
			break;

		case NEG_I32:
			fseek(mOutput, sizeof(int) * 1, SEEK_CUR);
			break;

		case MOV_REG_I32:
			fseek(mOutput, sizeof(int) * 2, SEEK_CUR);
			break;

		case MOV_MEM_REG_I32:
			fseek(mOutput, sizeof(int) * 3, SEEK_CUR);
			break;

		case MOV_REG_MEM_I32:
			fseek(mOutput, sizeof(int) * 3, SEEK_CUR);
			break;

		case JMP:
		case JZ:
		case JNZ:
			std::cout << "JUMP ";
			offset = ftell(mOutput);
			std::cout << offset << std::endl;
			fread(&temp[0], sizeof(int), 1, mOutput);
			std::cout << "\tVALUE OF (INSTR " << opcode << ")" << temp[0] << std::endl;
			temp[1] = GetLabelOffset(temp[0]);
			std::cout << "\tOFFSET TO " << temp[1] << std::endl;
			fseek(mOutput, offset, SEEK_SET);
			fwrite(&temp[1], sizeof(int), 1, mOutput);
			break;
		}
	}
}

// Process assembly line (disassemble single line)
void Disassembler::ProcessLine(const std::string& l)
{
	// Tokenize line
	std::string lt = l;
	StringUtil::trim(lt);
	std::vector<std::string> t = StringUtil::split(lt, ' ');
	for (std::string& s : t)
	{
		StringUtil::trim(s);
	}

	// Label
	if (t[0][t[0].length() - 1] == ':')
	{
		std::string label = t[0].substr(0, t[0].length() - 1);
		size_t offset = ftell(mOutput);
		StoreLabel(label, (int)offset);
		return;
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
	case CMPLEQ_I32:
	case CMPGEQ_I32:
	case CMPLESS_I32:
	case CMPGREATER_I32:
	case CMPEQ_I32:
	case CMPNEQ_I32:
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
		temp[1] = std::stoi(t[2]);
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

	case JMP:
	case JZ:
	case JNZ:
		temp[0] = GetLabel(t[1]);
		fwrite(&temp[0], sizeof(int), 1, mOutput);
		std::cout << "\tWRITING " << opcode << ", " << temp[0] << " at " << ftell(mOutput) << std::endl;
		break;
	}
}

// Constructor, pass in binary file and path to output file
Disassembler::Disassembler(const std::string& filename, const std::string& output)
{
	mOutputFilename = output;

	BuildOpcodes();
	
	mLabelsCount = 0;
	mLabels.clear();
	mLabelOffset.clear();

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

	fopen_s(&mOutput, mOutputFilename.c_str(), "rb+");

	ResolveLabels();

	fclose(mOutput);
}