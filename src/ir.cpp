// ir.cpp
// IR 处理程序
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
	// 节点树
	std::vector<Node*> Nodes;

	// IR 的 Lexer 规则
	std::list<LexerRule> matches =
	{
		LRULE(R"(\n)", "EOL"),
		LRULE(R"(DAT)", "DATA"),
		LRULE(R"(\d+)", "IDENTIFIER"),
		LRULE(R"([a-zA-Z_][a-zA-Z0-9_]*)", "IDENTIFIER"),
		LRULE(R"(,)", "COMMA"),
	};

	// 程序计数器
	uint32_t pc = 24;

	// IR 的 AST 规则
	std::list<ParserRule> rules =
	{
		ParserRule([&Nodes](LexerResult result) -> Option<int> {
			// 节点
			Node* node;
			std::string keyword(result.get(0).result.c_str());
			std::string para1(result.get(1).result.c_str());
			std::string para2(result.get(3).result.c_str());
			// 检查 keyword
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
			// 节点
				Node* node;
				std::string keyword(result.get(0).result.c_str());
				std::string para1(result.get(1).result.c_str());
				// 检查 keyword
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
			// 检查 keyword
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

	// 运行词法解析器
	Lexer lexer(code, matches);
	LexerResult result = lexer.tokenize();

	// 运行语法解析器
	Parser parser(result, rules);
	parser.parse();

	// 设置初始化信息
	VMInfo* vmInfo = reinterpret_cast<VMInfo*>(*mem);
	memset(vmInfo, 0, sizeof(VMInfo));
	vmInfo->vmMagicNumber = 0x24102410;
	vmInfo->vmVersion = 1;
	// 启用调试
	vmInfo->vmFlags = 0;
	vmInfo->vmDataRegionOffest = 24;
	vmInfo->vmCodeRegionOffest = pc;
	vmInfo->reserved3 = 0x91919191;

	// 开始编译
	for (auto& it : Nodes)
		// 返回指令长度 ( UweVM 采用 CISC 指令集 )
		pc += it->codegen(*mem + pc);

	// 最后必须添加用于退出的指令
	*((*mem) + pc) = 0xF0;
	pc++;

	// 返回长度
	return pc;
}