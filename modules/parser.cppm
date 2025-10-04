module;

#include <iostream>
#include <variant>
#include <functional>
#include <regex>

import lexer;
import logger;

export module parser;

/*
* Option ��
* ���𴴽�һ������ֵ������ֵ�����н��
* 
* ���캯����
*	Option(bool ok, T data)
*		ok: �Ƿ�ɹ�
*		data: ����
* 
* ������
*	const bool Ok() const
*		��ȡ���н��
*	const T& Data() const
*		��ȡ��������
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
* ParserRule ��
* �����﷨����
* 
* ���췽����
*	ParserRule(std::function<Option<int>(LexerResult)> trigger, std::list<std::string> rule)
*		trigger	: �հ�������
*		rule	: �����б� ( std::string Ϊ���ͱ�ʶ�� )
* ����: 
*	Option<int> press(std::list<std::string> yourRule, LexerResult result)
*		���Դ�����
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
	// NOTE: ���������ʽ�ǻ��ڹ����
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
		// ��ǰ�ҵ��Ľ��
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

			LexerResult(results);

			bool matched;

			for (auto& it : this->rule)
			{
				Option<int> ret = it.press(rule, result);
				if (ret.Ok()) 
					std::cout << "; Value: " << ret.Data() << std::endl;
				matched = ret.Ok();
			}

			if (!matched)
				Log.fatal("AST Error");
			pos++;
		}
	}
private:
	LexerResult result;
	std::list<ParserRule> rule;
	std::string endFlags;
};

// �����ڵ�
export class Node
{
public:
	// virtual int codegen() = 0;
	virtual int visit() = 0;
};

// �ӷ��ڵ�
class AddNode
	: public Node
{
public:
	AddNode(int n1, int n2)
	{
		this->n1 = n1;
		this->n2 = n2;
	}

	virtual int visit() override
	{
		printf("Result: %d\n", n1 + n2);
		return n1 + n2;
	}
private:
	int n1, n2;
};