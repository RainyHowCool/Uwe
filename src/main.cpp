#include <iostream>
#include <utility>
#include <regex>
#include <list>
#include <format>


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
#define RF(x) [](LexerResult result) -> Option<int>x
#define RRULE ParserRule

using namespace std;

int main()
{
	std::list<LexerRule> matches =
	{
		LRULE(R"(\n)", "EOL"),
		LRULE(R"(\d+)", "IDENTIFIER"),
		LRULE(R"([a-zA-Z_][a-zA-Z0-9_]*)", "IDENTIFIER"),
		LRULE(R"(,)", "COMMA"),
		//LRULE(R"("([^"\\]|\\.)*")", "STRING"),
		//LRULE(R"(=)", "ASSIGNMENT"),
		//LRULE(R"(:)", "COLON"),
		//LRULE(R"(\()", "LEFTPARENTHESIS"),
		//LRULE(R"(\))", "RIGHTPARENTHESIS"),
		//LRULE(R"(\+)", "ADD")
	};

	char* mem = reinterpret_cast<char*>(calloc(1024, 1024));
	char* oldMem = mem;
	VMInfo* vmInfo = reinterpret_cast<VMInfo*>(mem);
	vmInfo->vmMagicNumber = 0x24102410;
	vmInfo->vmVersion = 1;
	vmInfo->vmFlags = 1;
	vmInfo->vmDataRegionOffest = 0;
	vmInfo->vmCodeRegionOffest = 24;
	mem += 24;
	std::vector<Node*> Nodes;
	std::list<ParserRule> rules =
	{
		RRULE([&Nodes](LexerResult result) -> Option<int> {
				Node* node;
				std::string keyword(result.get(0).result.c_str());
				std::string para1(result.get(1).result.c_str());
				std::string para2(result.get(3).result.c_str());
				if (keyword == "add")
				{
					node = dynamic_cast<Node*>(new MathOpNode(para1, '+', atoi(para2.c_str())));
				}
				else
					Log.fatal("Bad command");
				Nodes.push_back(node);
				return Option<int>(true, 0);
			}, { "IDENTIFIER", "IDENTIFIER", "COMMA", "IDENTIFIER" })
	};

	Lexer lexer("mov r0, 114513\nadd r0, 1", matches);
	LexerResult result = lexer.tokenize();
	result.print();
	Parser parser(result, rules);
	parser.parse();
	Log.minOutputLevel = 1;
	//Log.trace("hi");
	//Log.debug("hi");
	//Log.info("hi");
	//Log.info("hello");
	//Log.note("hi");
	//Log.error("hi");
	//Log.warn("hi");
	//Log.fatal("hi");
	//Log.info("sO?");

	int memLen = 24;
	// 开始编译
	for (auto& it : Nodes)
	{
		int memNew = it->codegen(mem);
		memLen += memNew;
		mem += memNew;
	}

	*mem = 0xA2;
	mem += 1;
	memLen += 1;

	//unsigned int code[] = {
	//	0x24102410, 0, 0, 24, 1, 0, 0xBF5100A0, 0x00A10001, 1, 0xA2
	//};

	VMInstance vm((uint8_t*) oldMem, memLen);
	vm.run();

	return 0;
}
