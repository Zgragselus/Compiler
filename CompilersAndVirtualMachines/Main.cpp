///////////////////////////////////////////////////////////////////////////////
//
// This file is subject to the terms and conditions defined in
// file 'LICENSE.txt', which is part of this source code package.
//
///////////////////////////////////////////////////////////////////////////////
// (C) Vilem Otte <vilem.otte@post.cz>
///////////////////////////////////////////////////////////////////////////////

#include "Main.h"

class VirtualMachine
{
private:
	unsigned char* memory;		// VM memory
	int registers[4];			// VM registers (Reg 0, Reg 1, Instruction Pointer, Stack Pointer)

	enum
	{
		R0 = 0,
		R1,
		IP,
		SP
	};

	std::string registerName[4] =
	{
		"R0",
		"R1",
		"IP",
		"SP"
	};

public:
	// Constructor (specify how much memory you wish to alloc - defaults to 64K)
	VirtualMachine(size_t memorySize = 65536)
	{
		memory = (unsigned char*)malloc(memorySize);
	}

	// D-tor
	~VirtualMachine()
	{
		free(memory);
	}

	// Print out what is in registers (the ones we work with, not IP and SP)
	void DumpRegisters()
	{
		std::cout << "Registers:" << std::endl;
		std::cout << "\tr0 = " << registers[0] << std::endl;
		std::cout << "\tr1 = " << registers[1] << std::endl;
		std::cout << "\tip = " << registers[2] << std::endl;
		std::cout << "\tsp = " << registers[3] << std::endl;
		std::cout << std::endl;
	}

	// Print out stack from beginning address
	void DumpStack(size_t size)
	{
		std::cout << "Stack:" << std::endl;

		int* ptr = (int*)memory;
		ptr += size / sizeof(int);
		int position = size;
		int offset = 0;
		while (position < registers[SP])
		{
			std::cout << "\tsp + " << offset << " = " << *ptr << std::endl;
			ptr++;
			position += 4;
			offset += 4;
		}
		std::cout << std::endl;
	}

	// Execute the binary
	void Execute(const std::string& filename)
	{
		// Load binary into beginning of memory
		std::ifstream ifs(filename, std::ios::binary | std::ios::in);
		std::filebuf* pbuf = ifs.rdbuf();
		size_t size = (size_t)pbuf->pubseekoff(0, ifs.end, ifs.in);
		pbuf->pubseekpos(0, ifs.in);

		int* code = (int*)memory;
		pbuf->sgetn((char*)code, size);

		size_t instructionsCount = size / sizeof(int);
		registers[IP] = 0;			// Set IP to 0
		registers[SP] = size;		// Set SP to the end of code

		// While IP doesn't reach end of code, process instruction by instruction
		while ((size_t)(registers[IP]) < instructionsCount)
		{
			switch (code[registers[IP]])
			{
			case Disassembler::ADD_I32:
				std::cout << registers[IP] << " add.i32 " << registerName[code[registers[IP] + 1]] << " " << registerName[code[registers[IP] + 2]] << std::endl;
				registers[code[registers[IP] + 1]] += registers[code[registers[IP] + 2]];
				registers[IP] += 3;
				break;

			case Disassembler::SUB_I32:
				std::cout << registers[IP] << " sub.i32 " << registerName[code[registers[IP] + 1]] << " " << registerName[code[registers[IP] + 2]] << std::endl;
				registers[code[registers[IP] + 1]] -= registers[code[registers[IP] + 2]];
				registers[IP] += 3;
				break;

			case Disassembler::MUL_I32:
				std::cout << registers[IP] << " mul.i32 " << registerName[code[registers[IP] + 1]] << " " << registerName[code[registers[IP] + 2]] << std::endl;
				registers[code[registers[IP] + 1]] *= registers[code[registers[IP] + 2]];
				registers[IP] += 3;
				break;

			case Disassembler::DIV_I32:
				std::cout << registers[IP] << " div.i32 " << registerName[code[registers[IP] + 1]] << " " << registerName[code[registers[IP] + 2]] << std::endl;
				// Avoid division by 0 (print error and finish the program)
				if (registers[code[registers[IP] + 2]] == 0)
				{
					std::cout << "Error: Division by Zero, terminating application\n" << std::endl;
					registers[IP] = (int)instructionsCount;
					break;
				}
				registers[code[registers[IP] + 1]] /= registers[code[registers[IP] + 2]];
				registers[IP] += 3;
				break;

			case Disassembler::MOV_REG_REG:
				std::cout << registers[IP] << " mov.reg.reg " << registerName[code[registers[IP] + 1]] << " " << registerName[code[registers[IP] + 2]] << std::endl;
				registers[code[registers[IP] + 1]] = registers[code[registers[IP] + 2]];
				registers[IP] += 3;
				break;

			case Disassembler::PUSH_I32:
				std::cout << registers[IP] << " push.i32 " << registerName[code[registers[IP] + 1]] << std::endl;
				((int*)(memory + registers[SP]))[0] = registers[code[registers[IP] + 1]];
				registers[SP] += 4;
				registers[IP] += 2;
				break;

			case Disassembler::POP_I32:
				std::cout << registers[IP] << " pop.i32 " << registerName[code[registers[IP] + 1]] << std::endl;
				registers[SP] -= 4;
				registers[code[registers[IP] + 1]] = ((int*)(memory + registers[SP]))[0];
				registers[IP] += 2;
				break;

			case Disassembler::NEG_I32:
				std::cout << registers[IP] << " neg.i32 " << registerName[code[registers[IP] + 1]] << std::endl;
				registers[code[registers[IP] + 1]] = -registers[code[registers[IP] + 1]];
				registers[IP] += 2;
				break;

			case Disassembler::MOV_REG_I32:
				std::cout << registers[IP] << " mov.reg.i32 " << registerName[code[registers[IP] + 1]] << " " << code[registers[IP] + 2] << std::endl;
				registers[code[registers[IP] + 1]] = code[registers[IP] + 2];
				registers[IP] += 3;
				break;

			case Disassembler::MOV_MEM_REG_I32:
				std::cout << registers[IP] << " mov.mem.reg.i32 [" << registerName[code[registers[IP] + 1]] << " + " << code[registers[IP] + 2] << "] " << registerName[code[registers[IP] + 3]] << std::endl;
				((int*)(memory + registers[code[registers[IP] + 1]] + code[registers[IP] + 2]))[0] = registers[code[registers[IP] + 3]];
				registers[IP] += 4;
				break;

			case Disassembler::MOV_REG_MEM_I32:
			{
				std::cout << registers[IP] << " mov.reg.mem.i32 " << registerName[code[registers[IP] + 1]] << " [" << registerName[code[registers[IP] + 2]] << " + " << code[registers[IP] + 3] << "]" << std::endl;
				int reg1 = registers[code[registers[IP] + 2]];
				int offset = code[registers[IP] + 3];
				registers[code[registers[IP] + 1]] = ((int*)(memory + registers[code[registers[IP] + 2]] + code[registers[IP] + 3]))[0];
				registers[IP] += 4;
			}
				break;

			case Disassembler::CMPLEQ_I32:
				std::cout << registers[IP] << " cmpleq.i32 " << registerName[code[registers[IP] + 1]] << " " << registerName[code[registers[IP] + 2]] << std::endl;
				registers[R0] = (registers[code[registers[IP] + 1]] <= registers[code[registers[IP] + 2]]) ? 1 : 0;
				registers[IP] += 3;
				break;

			case Disassembler::CMPGEQ_I32:
				std::cout << registers[IP] << " cmpgeq.i32 " << registerName[code[registers[IP] + 1]] << " " << registerName[code[registers[IP] + 2]] << std::endl;
				registers[R0] = (registers[code[registers[IP] + 1]] >= registers[code[registers[IP] + 2]]) ? 1 : 0;
				registers[IP] += 3;
				break;

			case Disassembler::CMPLESS_I32:
				std::cout << registers[IP] << " cmpless.i32 " << registerName[code[registers[IP] + 1]] << " " << registerName[code[registers[IP] + 2]] << std::endl;
				registers[R0] = (registers[code[registers[IP] + 1]] < registers[code[registers[IP] + 2]]) ? 1 : 0;
				registers[IP] += 3;
				break;

			case Disassembler::CMPGREATER_I32:
				std::cout << registers[IP] << " cmpgreater.i32 " << registerName[code[registers[IP] + 1]] << " " << registerName[code[registers[IP] + 2]] << std::endl;
				registers[R0] = (registers[code[registers[IP] + 1]] > registers[code[registers[IP] + 2]]) ? 1 : 0;
				registers[IP] += 3;
				break;

			case Disassembler::CMPEQ_I32:
				std::cout << registers[IP] << " cmpeq.i32 " << registerName[code[registers[IP] + 1]] << " " << registerName[code[registers[IP] + 2]] << std::endl;
				registers[R0] = (registers[code[registers[IP] + 1]] == registers[code[registers[IP] + 2]]) ? 1 : 0;
				registers[IP] += 3;
				break;

			case Disassembler::CMPNEQ_I32:
				std::cout << registers[IP] << " cmpneq.i32" << registerName[code[registers[IP] + 1]] << " " << registerName[code[registers[IP] + 2]] << std::endl;
				registers[R0] = (registers[code[registers[IP] + 1]] != registers[code[registers[IP] + 2]]) ? 1 : 0;
				registers[IP] += 3;
				break;

			case Disassembler::JMP:
				std::cout << registers[IP] << " jmp " << (code[registers[IP] + 1] / 4) << std::endl;
				registers[IP] = (code[registers[IP] + 1] / 4);
				break;

			case Disassembler::JZ:
				std::cout << registers[IP] << " jz " << (code[registers[IP] + 1] / 4) << std::endl;
				if (registers[R0] == 0)
				{
					registers[IP] = (code[registers[IP] + 1] / 4);
				}
				else
				{
					registers[IP] += 2;
				}
				break;

			case Disassembler::JNZ:
				std::cout << registers[IP] << " jnz " << (code[registers[IP] + 1] / 4) << std::endl;
				if (registers[R0] != 0)
				{
					registers[IP] = (code[registers[IP] + 1] / 4);
				}
				else
				{
					registers[IP] += 2;
				}
				break;

			default:
				break;
			}
		}

		DumpStack(size);
		DumpRegisters();
	}
};

int main()
{
	std::chrono::time_point<std::chrono::system_clock> start, end;
	std::chrono::duration<double> elapsed_seconds;

	int x = 5;
	int y = 3;
	int z = x < y;

	//////////////////////////////////////////////////////////////////////////////
	// Preprocess source file (put includes into it, solve defines)
	std::vector<std::string> directories;
	directories.push_back("./");
	std::vector<std::string> defines;
	std::vector<std::string> data = Reader::ReadFile("script.scs");
	Preprocessor p(data, directories, defines, "script.scs");
	p.Save("Script_preprocessed.txt");

	std::cout << "Compiling source:" << std::endl;
	for (const std::string& s : data)
	{
		std::cout << s << std::endl;
	}
	std::cout << std::endl;

	//////////////////////////////////////////////////////////////////////////////
	// Perform lexical analysis on preprocessed file
	Lexer l = Lexer("Script_preprocessed.txt");
	l.SaveFile("Script_tokenized.txt");

	//////////////////////////////////////////////////////////////////////////////
	// Compilation into Assembly
	start = std::chrono::system_clock::now();
	Compiler c = Compiler(l, "Script_assembly.txt");
	c.Compile();
	end = std::chrono::system_clock::now();
	elapsed_seconds = end - start;
	std::cout << "Compilation took: " << elapsed_seconds.count() * 1000 << "ms\n";

	//////////////////////////////////////////////////////////////////////////////
	// Disassemble into machine code
	start = std::chrono::system_clock::now();
	Disassembler d = Disassembler("Script_assembly.txt", "Script_binary.scbin");
	d.Disassemble();
	end = std::chrono::system_clock::now();
	elapsed_seconds = end - start;
	std::cout << "Disassembly took: " << elapsed_seconds.count() * 1000 << "ms\n";

	//////////////////////////////////////////////////////////////////////////////
	// Execute machine code
	VirtualMachine v = VirtualMachine();
	start = std::chrono::system_clock::now();
	v.Execute("Script_binary.scbin");
	end = std::chrono::system_clock::now();
	elapsed_seconds = end - start;
	std::cout << "VM Execution took: " << elapsed_seconds.count() * 1000 << "ms\n";

	std::cin.get();

	return 0;
}