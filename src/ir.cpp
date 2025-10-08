// ir.cpp
// IR �������
// 2025 Uwelang
// 2025 RainyHowCool (xiaokuai)

#include <iostream>
#include <utility>
#include <regex>
#include <list>
#include <format>
#include <cstdint>

import lexer;
import parser;
import logger;
import vm;

#define LRULE(x, y) LexerRule(std::regex(x), y)

// �ڵ���
std::vector<Node*> Nodes;

// ���������
uint32_t pc;

// �ڴ�
char** mem = nullptr;

static std::list<LexerRule> getLexerRules()
{
	std::list<LexerRule> matches =
	{
		LRULE(R"(\n)", "EOL"),
		LRULE(R"(DAT)", "DATA"),
		LRULE(R"(\d+)", "IDENTIFIER"),
		LRULE(R"([a-zA-Z_][a-zA-Z0-9_]*)", "IDENTIFIER"),
		LRULE(R"(,)", "COMMA"),
	};
	return matches;
}

auto trigger1 = [](LexerResult result) -> Option<int> {
	Node* node = nullptr;
	std::string keyword(result.get(0).result.c_str());
	std::string para1(result.get(1).result.c_str());
	std::string para2(result.get(3).result.c_str());
	if (keyword == "add")
		node = new MathOpNode(para1, '+', atoi(para2.c_str()));
	else if (keyword == "sub")
		node = new MathOpNode(para1, '-', atoi(para2.c_str()));
	else if (keyword == "mul")
		node = new MathOpNode(para1, '*', atoi(para2.c_str()));
	else if (keyword == "div")
		node = new MathOpNode(para1, '/', atoi(para2.c_str()));
	else if (keyword == "mov")
		node = new MoveNode(para1, para2);
	else
		Log.fatal("Bad command");
	Nodes.push_back(node);
	return Option<int>(true, 0);
};

auto trigger2 = [](LexerResult result) -> Option<int> {
	Node* node = nullptr;
	std::string keyword(result.get(0).result.c_str());
	std::string para1(result.get(1).result.c_str());
	if (keyword == "push")
		node = new PushNode(para1);
	else if (keyword == "pop")
		node = new PopNode(para1);
	else if (keyword == "dat") {
		node = new DataNode(para1);
		pc += node->codegen(*mem + pc);
		return Option<int>(true, 0);
	}
	else
		Log.fatal("Bad command");
	Nodes.push_back(node);
	return Option<int>(true, 0);
};

auto trigger3 = [](LexerResult result) -> Option<int> {
	Node* node = nullptr;
	std::string keyword(result.get(0).result.c_str());
	if (keyword == "quit")
		node = new QuitNode();
	else if (keyword == "invoke")
		node = new InvokeNode();
	else
		Log.fatal("Bad command");
	Nodes.push_back(node);
	return Option<int>(true, 0);
};

static std::list<ParserRule> getASTRules()
{
	// IR �� AST ����
	std::list<ParserRule> rules =
	{
		ParserRule(trigger1, { "IDENTIFIER", "IDENTIFIER", "COMMA", "IDENTIFIER" }),
		ParserRule(trigger2, { "IDENTIFIER", "IDENTIFIER" }),
		ParserRule(trigger3, { "IDENTIFIER" })
	};

	return rules;
}

int generate_ir(std::string code, char** memory)
{
	// ��ʼ������
	Nodes.clear();
	pc = 24;

	// �����ڴ�
	mem = memory;

	// IR �� Lexer ����
	auto matches = getLexerRules();

	// IR �� AST ����
	auto rules = getASTRules();

	// ���дʷ�������
	Lexer lexer(code, matches);
	LexerResult result = lexer.tokenize();

	// �����﷨������
	Parser parser(result, rules);
	parser.parse();

	// ���ó�ʼ����Ϣ
	VMInfo* vmInfo = reinterpret_cast<VMInfo*>(*mem);
	memset(vmInfo, 0, sizeof(VMInfo));
	vmInfo->vmMagicNumber = 0x24102410;
	vmInfo->vmVersion = 1;
	// ���õ���
	vmInfo->vmFlags = 0;
	vmInfo->vmDataRegionOffest = 24;
	vmInfo->vmCodeRegionOffest = pc;
	vmInfo->reserved3 = 0x91919191;

	// ��ʼ����
	for (auto& it : Nodes)
		// ����ָ��� ( UweVM ���� CISC ָ� )
		pc += it->codegen(*mem + pc);

	// ��������������˳���ָ��
	*((*mem) + pc) = 0xF0;
	pc++;

	// ���س���
	return pc;
}