module;

#include <stdlib.h>
#include <stdio.h>
#include <vector>
#include <string>
#include <stdint.h>
#include <format>

import logger;


export module vm;

// 虚拟机基本信息
export struct VMInfo
{
	// 魔数
	uint32_t vmMagicNumber;
	// 版本号
	uint32_t vmVersion;
	// 常量偏移量
	uint32_t vmDataRegionOffest;
	// 字段偏移量
	uint32_t vmCodeRegionOffest;
	// 标志位
	uint8_t vmFlags;
	// reserved
	uint8_t ar1;
	uint16_t ar2;
	uint32_t ar3;
};

struct VMRegister
{
public:
	uint32_t r0;
	uint32_t r1;
	uint32_t r2;
	uint32_t r3;
	uint32_t r4;
	uint32_t pc = 0;
	uint32_t sp = 0;
	uint32_t bp = 0;
};

enum Opcode
{
	MOV = 0xA0, ADD = 0xA1, QUIT = 0xA2
};

export class VMInstance 
{
public:
	VMInstance(uint8_t* vmRaw, int sizeOfRaw, int vmMemSize = 256 * 1024 * 1024)
	{
		vmRegister = reinterpret_cast<VMRegister*>(calloc(sizeof(VMRegister), sizeof(VMRegister)));
		// 1. 加载虚拟机信息
		this->vmInfo = reinterpret_cast<VMInfo*>(vmRaw);
		// 2. 检查信息
		if (vmInfo->vmMagicNumber != 0x24102410)
			Log.fatal("Invaild Magic Number");
		else if (vmInfo->vmVersion > 1)
			Log.fatal("Invaild Version Number!");
		else if (vmInfo->vmFlags == 0x00000001)
			debug = true;
		// 2-1. 如果处于 Debug 模式，则输出头文件信息
		if (debug)
			printVMInfo(vmInfo);
		// 3. 分配虚拟机内存
		this->vmMemory = new uint8_t[vmMemSize];
		if (vmMemory == nullptr)
			Log.fatal(std::format("Alloc memory {} MiB failed!", vmMemSize / 1024 / 1024));
		if (debug)
			Log.debug(std::format("Alloced memory size: {} MiB", vmMemSize / 1024 / 1024));
		// 4. 拷贝原始数据
		this->vmMemSize = vmMemSize;
		// 5. 拷贝虚拟机数据
		memcpy(this->vmMemory, vmRaw, sizeOfRaw);
		char buffer[4096];
		sprintf(buffer, "%d bytes has been copyed to %#x from %#x", sizeof(vmRaw), reinterpret_cast<size_t>(vmRaw), reinterpret_cast<size_t>(vmMemory));
		Log.debug(buffer);
	}


	void run()
	{
		// 1. 加载分段
		code = vmMemory + vmInfo->vmCodeRegionOffest;
		data = vmMemory + vmInfo->vmDataRegionOffest;
		vmRegister->pc = 0;
		// 2. 执行代码
		for (;;)
		{
			switch (readCode())
			{
			case MOV:
			{
				vmRegister->pc++;
				uint8_t regBit = readCode();
				// 获取模式（当前字符前两位，11为寄存器相加，00为寄存器加数值）
				uint8_t mode = regBit >> 6;
				// 获取次寄存器（当前字符第3到第5位）
				uint32_t* secReg = getRegister(regBit & 0b00111000 >> 3);
				// 获取主寄存器（最后三位）
				uint32_t* firSeg = getRegister(regBit & 0b00000111);
				int imm = 255;
				if (mode == 00) { imm = readInt(vmRegister->pc++); *firSeg = imm; }
				else if (mode == 11) *firSeg = *secReg;
				else Log.fatal("MOV: Unmatched mode");
				Log.debug(std::format("Register {} is imm {}\nR0: {}", regBit, imm, vmRegister->r0));
				break;
			}
			case ADD: 
			{
				vmRegister->pc++;
				uint8_t regBit = readCode();
				// 获取模式（当前字符前两位，11为寄存器相加，00为寄存器加数值）
				uint8_t mode = regBit >> 6;
				// 获取次寄存器（当前字符第3到第5位）
				uint32_t* secReg = getRegister(regBit & 0b00111000 >> 3);
				// 获取主寄存器（最后三位）
				uint32_t* firSeg = getRegister(regBit & 0b00000111);
				int imm = 255;
				if (mode == 00) { imm = readInt(vmRegister->pc++); *firSeg += imm; }
				else if (mode == 11) *firSeg += *secReg;
				else Log.fatal("ADD: Unmatched mode");
				Log.debug(std::format("Register {} adds imm {}\nR0: {}", regBit, imm, vmRegister->r0));
				break;
			}
			case QUIT:
				Log.info("Bye!");
				exit(0);
			}
			vmRegister->pc++;
		}
	}
private:
	uint8_t readCode(int _before = 0)
	{
		return *reinterpret_cast<uint8_t*>(reinterpret_cast<size_t>(code) + this->vmRegister->pc);
	}


	uint32_t* getRegister(uint8_t reg)
	{
		switch (reg)
		{
		case 0b000: return &vmRegister->r0;
		case 0b001: return &vmRegister->r1;
		case 0b010: return &vmRegister->r2;
		case 0b011: return &vmRegister->r3;
		case 0b100: return &vmRegister->r4;
		case 0b101: return &vmRegister->pc;
		case 0b110: return &vmRegister->sp;
		case 0b111: return &vmRegister->bp;
		}
		Log.fatal("Wrong register");
		return nullptr;
	}

	uint32_t readInt(int _before = 0)
	{
		int res = 0;
		res = readCode();
		res += readCode(this->vmRegister->pc++) << 8;
		res += readCode(this->vmRegister->pc++) << 16;
		res += readCode(this->vmRegister->pc++) << 24;
		return res;
	}

	static void printVMInfo(VMInfo* vmInfo)
	{
		char buffer[4096];
		Log.debug("UweVM Header Information\n");
		Log.debug("==========================\n");
		sprintf(buffer, "\tMagic Number:\t %#x\n", vmInfo->vmMagicNumber);
		Log.debug(buffer);
		sprintf(buffer, "\tVM Version:\t %d\n", vmInfo->vmVersion);
		Log.debug(buffer);
		sprintf(buffer, "\tVM Flags:\t %#x\n", vmInfo->vmFlags);
		Log.debug(buffer);
		sprintf(buffer, "\tData Offest:\t %d\n", vmInfo->vmDataRegionOffest);
		Log.debug(buffer);
		sprintf(buffer, "\tCode Offest:\t %d\n", vmInfo->vmCodeRegionOffest);
		Log.debug(buffer);
	}

	bool debug = false;

	VMInfo* vmInfo;
	VMRegister* vmRegister;

	int vmMemSize = 0;
	int vmHeapStart = 0;

	uint8_t* vmMemory = nullptr;

	uint8_t* code;
	uint8_t* data;
};