#include <iostream>
#include <utility>
#include <regex>
#include <list>


import lexer;
import parser;
import logger;

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
		LRULE(R"(;)", "EOL"),
		LRULE(R"(\d+)", "INTEGER"),
		LRULE(R"([a-zA-Z_][a-zA-Z0-9_]*)", "IDENTIFIER"),
		LRULE(R"("([^"\\]|\\.)*")", "STRING"),
		LRULE(R"(=)", "ASSIGNMENT"),
		LRULE(R"(:)", "COLON"),
		LRULE(R"(\()", "LEFTPARENTHESIS"),
		LRULE(R"(\))", "RIGHTPARENTHESIS"),
		LRULE(R"(\+)", "ADD")
	};

	std::list<ParserRule> rules =
	{
		RRULE(RF({
				int value1 = atoi(result.get(0).result.c_str());
				int value2 = atoi(result.get(2).result.c_str());
				return Option<int>(true, value1 + value2);
			}), { "INTEGER", "ADD", "INTEGER" })
	};

	Lexer lexer("1+3", matches);
	LexerResult result = lexer.tokenize();
	result.print();
	Parser parser(result, rules);
	parser.parse();
	/*
	Log.minOutputLevel = 1;
	Log.trace("hi");
	Log.debug("hi");
	Log.info("hi");
	Log.info("hello");
	Log.note("hi");
	Log.error("hi");
	Log.warn("hi");
	Log.fatal("hi");
	Log.info("sO?");
	*/
	return 0;
}
