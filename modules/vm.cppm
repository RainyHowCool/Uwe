module;

#include <stdlib.h>
#include <stdio.h>
#include <vector>
#include <string>
#include <stdint.h>

export module vm;

enum class VMConstantType {
	STRING, DIGIT, LLONG, IDENTIFIER
};

// �����������Ϣ
struct VMInfo
{
	// ħ��
	uint32_t vmMagicNumber;
	// �汾��
	uint32_t vmVersion;
	// ��־λ
	uint8_t vmFlags;
	// �޶��汾��
	uint8_t vmPatchVersion;
	// ��������
	uint16_t vmReserved1;
	// ����ƫ����
	uint32_t vmConstantOffest;
	// �ֶ�ƫ����
	uint32_t vmFieldOffest;
	// �����ش�С
	uint32_t vmConstantSize;
	// �ֶδ�С
	uint32_t vmFieldSize;
};

struct VMConstant
{
	VMConstantType type =  VMConstantType::DIGIT;
	std::string identifier = "";
	int digit = 0;
	long long llong = 0;
};

class VMInstance 
{
public:
	VMInstance(void* vmRaw)
	{
		std::vector<VMConstant> constantPool;
		// 1. �����������Ϣ
		VMInfo* vmInfo = (VMInfo*) vmRaw;
		// 2. �����Ϣ
		if (vmInfo->vmMagicNumber != 0x24102410)
			vm_crash("Invaild Magic Number");
		else if (vmInfo->vmVersion > 1)
			vm_crash("Invaild Version Number!");
		else if (vmInfo->vmFlags == 0x00000001)
			debug = 1;
		else if (vmInfo->vmPatchVersion > 0)
			vm_warn("Not install patch!");
		else if (vmInfo->vmConstantOffest == 0)
			vm_crash("Invaild Constant Offest!");
		else if (vmInfo->vmFieldOffest == 0)
			vm_crash("Invaild Field Offest!");
		// 2-1. ������� Debug ģʽ�������ͷ�ļ���Ϣ
		if (debug) {
			printf("UweVM Header Information\n");
			printf("==========================\n");
			printf("\tMagic Number:\t %#x\n", vmInfo->vmMagicNumber);
			printf("\tVM Version:\t %d\n", vmInfo->vmVersion);
			printf("\tVM Flags:\t %#x\n", vmInfo->vmFlags);
			printf("\tPatch Version:\t %d\n", vmInfo->vmPatchVersion);
			printf("\tConstant Offest:\t %d\n", vmInfo->vmConstantOffest);
			printf("\tField Offest:\t %d\n", vmInfo->vmConstantOffest);
			printf("\tConstant Size:\t: %d\n", vmInfo->vmConstantSize);
			printf("\tField Size:\t %d\n", vmInfo->vmFieldSize);
			printf("\n");
		}
		// 3. ���س�����
		char* constantPoolPtr = (char*)((size_t)vmRaw + vmInfo->vmConstantSize);
		VMConstant constant;
		VMConstantType type;
		int digit;
		long long llong;
		std::string str;
		switch (*constantPoolPtr) {
		case 1:
			type = VMConstantType::DIGIT;
			constantPoolPtr++;
			digit = *(int*)constantPoolPtr;
			constantPoolPtr += sizeof(int);
			break;
		case 2:
			type = VMConstantType::LLONG;
			constantPoolPtr++;
			break;
		case 3:
			type = VMConstantType::IDENTIFIER;
			constantPoolPtr++;
			break;
		case 4:
			type = VMConstantType::STRING;
			constantPoolPtr++;
			break;
		}
	}

	static void vm_crash(const char* crashInfo) 
	{
		fprintf(stderr, "VM Crashed because %s\nExited\n", crashInfo);
		exit(1);
	}

	static void vm_warn(const char* warnInfo)
	{
		fprintf(stderr, "warn: %s\n", warnInfo);
	}
private:
	int debug = 0;
};