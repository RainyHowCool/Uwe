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

#define LRULE(x, y) LexerRule(std::regex(x), y)

// 节点树
std::vector<Node*> Nodes;

// 程序计数器
uint32_t pc;

// 内存
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
	// IR 的 AST 规则
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
	// 初始化变量
	Nodes.clear();
	pc = 24;

	// 设置内存
	mem = memory;

	// IR 的 Lexer 规则
	auto matches = getLexerRules();

	// IR 的 AST 规则
	auto rules = getASTRules();

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