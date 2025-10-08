module;

#include <iostream>
#include <variant>
#include <functional>
#include <regex>

export module parser;

import lexer;
import logger;

/*
* Option 类
* 负责创建一个返回值，包含值和运行结果
* 
* 构造函数：
*	Option(bool ok, T data)
*		ok: 是否成功
*		data: 数据
* 
* 方法：
*	const bool Ok() const
*		获取运行结果
*	const T& Data() const
*		获取返回数据
*/
export template <typename T>
class Option {
public:
	Option(bool ok, T data)
	{
		this->ok = ok;
		this->data = data;
	}

	const bool Ok() const
	{
		return ok;
	}

	const T& Data() const
	{
		return data;
	}
private:
	bool ok;
	T data;
};

/*
* ParserRule 类
* 单条语法规则
* 
* 构造方法：
*	ParserRule(std::function<Option<int>(LexerResult)> trigger, std::list<std::string> rule)
*		trigger	: 闭包触发器
*		rule	: 规则列表 ( std::string 为类型标识符 )
* 方法: 
*	Option<int> press(std::list<std::string> yourRule, LexerResult result)
*		测试触发器
*/

export class ParserRule {
public:
	ParserRule()
	{}

	ParserRule(std::function<Option<int>(LexerResult)> trigger, std::list<std::string> rule)
	{
		this->trigger = trigger;
		this->rule = rule;
	}

	Option<int> press(std::list<std::string> yourRule, LexerResult result)
	{
		if (rule == yourRule)
		{
			return trigger(result);
		}
		return Option<int>(false, -1);
	}
private:
	std::function<Option<int>(LexerResult)> trigger;
	std::list<std::string> rule;
};

export class Parser
{
public:
	// NOTE: 这个正则表达式是基于规则的
	Parser(LexerResult result, std::list<ParserRule> rule, std::string endFlags = "EOL")
	{
		this->result = result;
		this->rule = rule;
		this->endFlags = endFlags;
	}

	void parse()
	{
		size_t pos = 0;
		size_t len = result.size();
		// 当前找到的结果
		std::vector<MatchResult> results;
		std::list<std::string> rule;

		while (pos < len)
		{
			if (result.get(pos).type != endFlags)
			{
				MatchResult mr = result.get(pos);
				rule.push_back(mr.type);
				results.push_back(mr);
				pos++;
				continue;
			}

			LexerResult result2 = LexerResult(results);

			bool matched;

			for (auto& it : this->rule)
			{
				Option<int> ret = it.press(rule, result2);
				matched = ret.Ok();
				if (ret.Ok())
					break;
			}

			//if (!matched)
				//Log.fatal("AST Error");
			results.clear();
			rule.clear();
			pos++;
		}
	}
private:
	LexerResult result;
	std::list<ParserRule> rule;
	std::string endFlags;
};

// 基础节点
export class Node
{
public:
	virtual int codegen(char* mem) = 0;
};

unsigned char getRegisterByString(std::string nreg)
{
	unsigned char reg = 0;
	if (nreg == "r0")
		reg = 0;
	else if (nreg == "r1")
		reg = 1;
	else if (nreg == "r2")
		reg = 2;
	else if (nreg == "r3")
		reg = 3;
	else if (nreg == "ss")
		reg = 4;
	else if (nreg == "sp")
		reg = 6;
	else if (nreg == "bp")
		reg = 7;
	return reg;
}

// 加法节点
export class MathOpNode
	: public Node
{
public:
	MathOpNode(std::string nreg, char sym, int n)
	{
		this->nreg = nreg;
		this->n = n;
		this->sym = sym;
	}

	virtual int codegen(char* mem) override
	{
		unsigned char reg = getRegisterByString(nreg);
		switch (sym)
		{
		case '+':
			mem[0] = 0xA1;
			break;
		case '-':
			mem[0] = 0xA2;
			break;
		case '*':
			mem[0] = 0xA3;
			break;
		case '/':
			mem[0] = 0xA4;
			break;
		}

		mem[1] = reg;
		*reinterpret_cast<int*>(mem + 2) = n;

		return 6;
	}
private:
	std::string nreg;
	int n;
	char sym;
};

export class MoveNode
	: public Node
{
public:
	MoveNode(std::string nreg1, std::string nreg2)
	{
		this->nreg1 = nreg1;
		this->nreg2 = nreg2;
	}

	virtual int codegen(char* mem) override
	{
		mem[0] = 0xA0;
		mem[1] = 0b11000000;
		unsigned char reg1 = getRegisterByString(nreg1);
		unsigned char reg2 = getRegisterByString(nreg2);
		mem[1] = mem[1] | (reg1 << 3) | reg2;
		return 2;
	}
private:
	std::string nreg1;
	std::string nreg2;
};

export class PushNode
	: public Node
{
public:
	PushNode(std::string nreg)
	{
		this->nreg = nreg;
	}
	virtual int codegen(char* mem) override
	{
		mem[0] = 0xA5;
		unsigned char reg = getRegisterByString(nreg);
		mem[1] = reg;
		return 2;
	}
private:
	std::string nreg;
};

export class PopNode
	: public Node
{
public:
	PopNode(std::string nreg)
	{
		this->nreg = nreg;
	}
	virtual int codegen(char* mem) override
	{
		mem[0] = 0xA6;
		unsigned char reg = getRegisterByString(nreg);
		mem[1] = reg;
		return 2;
	}
private:
	std::string nreg;
};

export class DataNode
	: public Node
{
public:
	DataNode(std::string str)
	{
		this->str = str;
	}
	virtual int codegen(char* mem) override
	{
		auto len = str.length();
		memcpy(mem, str.c_str(), len);
		mem[len] = 0;
		return len + 1;
	}
private:
	std::string str;
};

export class InvokeNode
	: public Node
{
public:
	InvokeNode()
	{
	}

	virtual int codegen(char* mem) override
	{
		mem[0] = 0xF1;
		return 1;
	}
};

export class QuitNode
	: public Node
{
public:
	QuitNode()
	{
	}

	virtual int codegen(char* mem) override
	{
		mem[0] = 0xF0;
		return 1;
	}
};