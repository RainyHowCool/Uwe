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

/*
#define MatchResult std::vector<std::pair<std::string, std::string>>
typedef std::vector<std::pair<std::regex*, std::string>> LexerRule;
typedef std::vector<std::pair<std::function<int(MatchResult)>, std::vector<std::string>>> ParserRule;
*/

#define LRULE(x, y) LexerRule(std::regex(x), y)
#define RRULE ParserRule

using namespace std;

int generate_ir(std::string code, char** mem)
{
	// �ڵ���
	std::vector<Node*> Nodes;

	// IR �� Lexer ����
	std::list<LexerRule> matches =
	{
		LRULE(R"(\n)", "EOL"),
		LRULE(R"(DAT)", "DATA"),
		LRULE(R"(\d+)", "IDENTIFIER"),
		LRULE(R"([a-zA-Z_][a-zA-Z0-9_]*)", "IDENTIFIER"),
		LRULE(R"(,)", "COMMA"),
	};

	// ���������
	uint32_t pc = 24;

	// IR �� AST ����
	std::list<ParserRule> rules =
	{
		ParserRule([&Nodes](LexerResult result) -> Option<int> {
			// �ڵ�
			Node* node;
			std::string keyword(result.get(0).result.c_str());
			std::string para1(result.get(1).result.c_str());
			std::string para2(result.get(3).result.c_str());
			// ��� keyword
			if (keyword == "add")
				node = dynamic_cast<Node*>(new MathOpNode(para1, '+', atoi(para2.c_str())));
			else if (keyword == "sub")
				node = dynamic_cast<Node*>(new MathOpNode(para1, '-', atoi(para2.c_str())));
			else if (keyword == "mul")
				node = dynamic_cast<Node*>(new MathOpNode(para1, '*', atoi(para2.c_str())));
			else if (keyword == "div")
				node = dynamic_cast<Node*>(new MathOpNode(para1, '/', atoi(para2.c_str())));
			else if (keyword == "mov")
				node = dynamic_cast<Node*>(new MoveNode(para1, para2));
			else
				Log.fatal("Bad command");

			Nodes.push_back(node);
			return Option<int>(true, 0);
		}, { "IDENTIFIER", "IDENTIFIER", "COMMA", "IDENTIFIER" }),
	ParserRule([&Nodes,&mem, &pc](LexerResult result) -> Option<int> {
			// �ڵ�
				Node* node;
				std::string keyword(result.get(0).result.c_str());
				std::string para1(result.get(1).result.c_str());
				// ��� keyword
				if (keyword == "push")
					node = dynamic_cast<Node*>(new PushNode(para1));
				else if (keyword == "pop")
					node = dynamic_cast<Node*>(new PopNode(para1));
				else if (keyword == "dat") {
					node = dynamic_cast<Node*>(new DataNode(para1));
					pc += node->codegen(*mem + pc);
					return Option<int>(true, 0);
				}
				else
					Log.fatal("Bad command");
				Nodes.push_back(node);
				return Option<int>(true, 0);
		}, { "IDENTIFIER", "IDENTIFIER" }),
	ParserRule([&Nodes](LexerResult result) -> Option<int> {
			// Node
			Node* node;
			std::string keyword(result.get(0).result.c_str());
			// ��� keyword
			if (keyword == "quit")
				node = dynamic_cast<Node*>(new QuitNode());
			else if (keyword == "invoke")
				node = dynamic_cast<Node*>(new InvokeNode());
			else
				Log.fatal("Bad command");
			Nodes.push_back(node);
			return Option<int>(true, 0);
		}, { "IDENTIFIER" })
	};

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