module;

#include <stdlib.h>
#include <stdio.h>
#include <vector>
#include <string>
#include <stdint.h>
#include <format>
#include <iostream>

export module vm;

import logger;

// �����������Ϣ
export struct VMInfo
{
	// ħ��
	uint32_t vmMagicNumber;
	// �汾��
	uint32_t vmVersion;
	// ����ƫ����
	uint32_t vmDataRegionOffest;
	// �ֶ�ƫ����
	uint32_t vmCodeRegionOffest;
	// ��־λ
	uint8_t vmFlags;
	// ����λ ( Ϊ�� 4 �ֽ� ���� )
	uint8_t reserved1;
	uint16_t reserved2;
	uint32_t reserved3;
};

// �Ĵ����ṹ��
struct VMRegister
{
public:
	uint32_t r0;
	uint32_t r1;
	uint32_t r2;
	uint32_t r3;
	uint32_t ss = 0;
	uint32_t pc = 0;
	uint32_t sp = 0;
	uint32_t bp = 0;
};

// �ֽ���
enum Opcode
{
	MOV = 0xA0, ADD = 0xA1, SUB = 0xA2, MUL = 0xA3, DIV = 0xA4, PUSH = 0xA5, POP = 0xA6, QUIT = 0xF0
};

// �����ʵ��
export class VMInstance 
{
public:
	VMInstance(uint8_t* vmRaw, int sizeOfRaw, int vmMemSize = 4 * 1024 * 1024)
	{
		vmRegister = reinterpret_cast<VMRegister*>(calloc(sizeof(VMRegister), sizeof(VMRegister)));
		// 1. �����������Ϣ
		this->vmInfo = reinterpret_cast<VMInfo*>(vmRaw);
		// 2. �����Ϣ
		if (vmInfo->vmMagicNumber != 0x24102410)
			Log.fatal("Invaild Magic Number");
		else if (vmInfo->vmVersion > 1)
			Log.fatal("Invaild Version Number!");
		else if (vmInfo->vmFlags == 0x00000001)
			debug = true;
		// 2-1. ������� Debug ģʽ�������ͷ�ļ���Ϣ
		if (debug)
			printVMInfo(vmInfo);
		// 3. ����������ڴ�
		this->vmMemory = new uint8_t[vmMemSize];
		if (vmMemory == nullptr)
		{
			fprintf(stderr, "VM: Alloc memory %d MiB failed!\n", vmMemSize / 1024 / 1024);
			exit(-1);
		}
		if (debug)
			printf("VM: Alloced memory size: %d MiB\n", vmMemSize / 1024 / 1024);
		// 4. ����ԭʼ����
		this->vmMemSize = vmMemSize;
		// 5. �������������
		memcpy(this->vmMemory, vmRaw, sizeOfRaw);
		printf("VM: %zu bytes has been copyed to %#zx from %#zx\n", sizeof(vmRaw), reinterpret_cast<size_t>(vmRaw), reinterpret_cast<size_t>(vmMemory));
		// 6. ����ջ��ʼλ��
		vmRegister->sp = vmMemSize;
	}


	void run()
	{
		// 1. ��ʼ������
		int mode = 0;
		uint32_t* firReg = nullptr;
		uint32_t* secReg = nullptr;
		int imm = 0;
		printf("VM: One-Step Debug enabled\n\n");
		vmRegister->ss = vmInfo->vmCodeRegionOffest;
		// 2. ִ�д���
		for (;;)
		{
			oneStepDebug();
			// TODO: �����ٻ���
			switch (readCode())
			{
			case MOV:
				getArguments(&mode, &firReg, &secReg, &imm, vmRegister->pc++);
				if (mode == 0b0) *firReg = imm;
				else if (mode == 0b11) { *firReg = *secReg; vmRegister->pc -= 4; }
				else Log.fatal("MOV: Unmatched mode");
				break;
			case ADD: 
				getArguments(&mode, &firReg, &secReg, &imm, vmRegister->pc++);
				if (mode == 0b0) *firReg += imm;
				else if (mode == 0b11) { *firReg += *secReg; vmRegister->pc -= 4; }
				else Log.fatal("ADD: Unmatched mode");
				break;
			case SUB:
				getArguments(&mode, &firReg, &secReg, &imm, vmRegister->pc++);
				if (mode == 0b0) *firReg -= imm;
				else if (mode == 0b11) { *firReg -= *secReg; vmRegister->pc -= 4; }
				else Log.fatal("SUB: Unmatched mode");
				break;
			case MUL:
				getArguments(&mode, &firReg, &secReg, &imm, vmRegister->pc++);
				if (mode == 0b0) *firReg *= imm;
				else if (mode == 0b11) { *firReg *= *secReg; vmRegister->pc -= 4; }
				else Log.fatal("MUL: Unmatched mode");
				break;
			case DIV:
				getArguments(&mode, &firReg, &secReg, &imm, vmRegister->pc++);
				if (mode == 0b0) *firReg /= imm;
				else if (mode == 0b11) { *firReg /= *secReg; vmRegister->pc -= 4; }
				else Log.fatal("DIV: Unmatched mode");
				break;
			case PUSH:
				vmRegister->sp -= 4;
				*(uint32_t*)(vmMemory + vmRegister->sp) = *getRegister(readCode(vmRegister->pc++));
				break;
			case POP:
				*getRegister(readCode(vmRegister->pc++)) = *(uint32_t*)(vmMemory + vmRegister->sp);
				vmRegister->sp += 4;
				break;
			case QUIT:
				printf("VM: Program quited\n");
				oneStepDebug();
				exit(vmRegister->r0);
			}
			vmRegister->pc++;
		}
	}
private:
	// �ӵ�ǰָ��λ�ü��� 1 �ֽ�����
	uint8_t readCode(int _before = 0)
	{
		return *reinterpret_cast<uint8_t*>(reinterpret_cast<size_t>(vmMemory + vmRegister->ss) + this->vmRegister->pc);
	}

	void getArguments(int* mode, uint32_t** firReg, uint32_t** secReg, int *imm, int before = 0)
	{
		uint8_t regBit = readCode();
		// ��ȡģʽ ( ��ǰ�ַ�ǰ��λ��11 Ϊ�Ĵ�����ӣ�00 Ϊ�Ĵ�������ֵ )
		*mode = regBit >> 6;
		// ��ȡ�μĴ��� ( ��ǰ�ַ��� 3 ���� 5 λ )
		*secReg = getRegister(regBit & 0b00111000 >> 3);
		// ��ȡ���Ĵ��� ( ��� 3 λ )
		*firReg = getRegister(regBit & 0b00000111);
		*imm = readInt(vmRegister->pc++);
	}

	// ��ȡ�Ĵ�����ָ��
	uint32_t* getRegister(uint8_t reg)
	{
		switch (reg)
		{
		case 0b000: return &vmRegister->r0;
		case 0b001: return &vmRegister->r1;
		case 0b010: return &vmRegister->r2;
		case 0b011: return &vmRegister->r3;
		case 0b100: return &vmRegister->ss;
		case 0b101: return &vmRegister->pc;
		case 0b110: return &vmRegister->sp;
		case 0b111: return &vmRegister->bp;
		}
		Log.fatal("Wrong register");
		return nullptr;
	}

	// �ӵ�ǰָ��λ�ü��� 4 �ֽ�����
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
		printf("VM: Debug enabled\n");
		printf("UweVM Header Information\n");
		printf("==========================\n");
		printf("\tMagic Number:\t %#x\n", vmInfo->vmMagicNumber);
		printf("\tVM Version:\t %d\n", vmInfo->vmVersion);
		printf("\tVM Flags:\t %#x\n", vmInfo->vmFlags);
		printf("\tData Offest:\t %d\n", vmInfo->vmDataRegionOffest);
		printf("\tCode Offest:\t %d\n", vmInfo->vmCodeRegionOffest);
	}

	// ��ӡ�Ĵ�����Ϣ
	void printAllRegisters()
	{
		char buffer[4096];
		sprintf(buffer, "\tr0: %#x(%d) r1: %#x(%d) r2: %#x(%d) r3: %#x(%d)\n\tss: %#x(%d) pc: %#x(%d) sp: %#x(%d) bp: %#x(%d)\n",
			vmRegister->r0, vmRegister->r0, vmRegister->r1, vmRegister->r1, vmRegister->r2, vmRegister->r2,
			vmRegister->r3, vmRegister->r3, vmRegister->ss, vmRegister->ss, vmRegister->pc, vmRegister->pc,
			vmRegister->sp, vmRegister->sp, vmRegister->bp, vmRegister->bp);
		printf("All registers\n===============\n%s", buffer);
	}

	// ��������
	void oneStepDebug(bool breakpoint = false)
	{
		if ((!debug || skipOneStepDebug) && !breakpoint) return;
		printAllRegisters();
		printf("\n");
		for (;;)
		{
			std::string in;
			printf("(%#x) ", vmRegister->ss + vmRegister->pc);
			fflush(stdout);
			std::cin >> in;
			if (in == "r") printAllRegisters();
			else if (in == "n") break;
			else if (in == "s") { skipOneStepDebug = true; break; }
			else if (in == "p") printf("%#x + %#x: %#x", vmRegister->ss, vmRegister->pc, *(vmMemory + vmRegister->ss + vmRegister->pc));
			else printf("Bad command %s", in.c_str());
			printf("\n");
		}
	}

	bool debug = false;
	bool skipOneStepDebug = false;

	VMInfo* vmInfo;
	VMRegister* vmRegister;

	int vmMemSize = 0;
	int vmHeapStart = 0;

	uint8_t* vmMemory = nullptr;
};