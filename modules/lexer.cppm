module;

#include <string>
#include <format>
#include <iostream>
#include <regex>
#include <list>
#include <unordered_set>
#include <string_view>

export module lexer;

import logger;

export class LexerRule 
{
public:
    LexerRule(std::regex match, std::string type)
    {
        this->match = match;
        this->type = type;
    }

    LexerRule(std::regex match, const char* type)
    {
        this->match = match;
        this->type = type;
    }

    std::regex match;
    std::string type;
};

export class MatchResult 
{
public:
    MatchResult(std::string result, std::string type)
    {
        this->result = result;
        this->type = type;
    }
    std::string result;
    std::string type;
};

export class LexerResult 
{
public:
    LexerResult() 
    {
        this->result = { MatchResult("", std::string("NULL")) };
    }

    LexerResult(std::vector<MatchResult> result)
    {
        this->result = result;
    }

    MatchResult& get(size_t index)
    {
        return result[index];
    }

    const size_t size() const
    {
        return result.size();
    }

    /* NOTE: ���һ��ܿ���ʱ����һ����������Ĺ���
    * ���ȣ���������ᴴ��һ���ַ������ڴ�Ž�����ɵ�����
    * ws ��������ո�, tab �������� tab, tab2 ��������˫�ո�, �������������� withTab ��������( �Ƿ��ʽ����� ), result �Դ� '{' �ַ�
    * ���������ʽ�����������Ὣ�ַ���ת��������ĸ�ʽ
    * ---- �ֽ��� ----
    * {}( �� Tab )[
    *   {}( ˫ Tab ) "result":{}( �ո�λ�� )"{}( �����Ҳ����ƥ�䵽���ַ��� )",
    *   {}( ˫ Tab ) "type":{}( �ո�λ�� )"{}( ���ͣ����� INTEGER, IDENTIFIER �� )
    * {}( �� Tab )]
    * ---- �ֽ��� ----
    * �ڽ�β�Ķ�����, result �� withTab ����, ������ø�ʽ������������ǰ���뻻�з�
    * ������ʲô�����result�������������� ',' �ַ�
    * �����������ַ��� "name:John" ����ȷ������ IDENTIFIER �� COLON ��������ʽ�������Ǹ�ʽ���������:
    * ---- �ֽ��� ----
    * {
    *   [
    *       "result": "name",
    *       "type": "IDENTIFIER"
    *   ],
    *   [
    *       "result": ":",
    *       "type": "COLON"
    *   ],
    *   [
    *       "result": "John",
    *       "type": "IDENTIFIER"
    *   ]
    * }
    * ---- �ֽ��� ----
    * ����û�б���ʽ��������(ֻ������һ��):
    * {["result":"name","type":"IDENTIFIER"],["result":":","type":"COLON"],["result":"John","type":"IDENTIFIER"]}
    * ϣ����һ��ע������������⴮����
    */
    std::string get_string(bool withTab = false)
    {
        // INTERESTING IDEA
        std::string tab = withTab ? "\n\t" : "";
        std::string tab2 = withTab ? tab + "\t" : "";
        std::string ws /* White Space */ = withTab ? " " : "";
        std::string result = "{";
        // �������
        for (const auto& it : this->result)
            result += std::format("{}[{}\"result\":{}\"{}\",{}\"type\":{}\"{}\"{}],",
                tab, tab2, ws, it.result, tab2, ws, it.type, tab);
        // ���ǰ��� ',' �ַ�
        result = result.substr(0, result.length() - 1);
        result += withTab ? "\n" : "";
        result += "}";
        return result;
    }

    void print(bool withTab = true, bool withoutNL = false)
    {
        std::cout << get_string(withTab) << (withoutNL ? "" : "\n");
    }
private:
    std::vector<MatchResult> result;
};

export class Lexer
{
public:
    Lexer(std::string str, std::list<LexerRule> matches,
          std::vector<char> whiteSpaceList = { '\t', '\r', ' ' },
          std::string endFlags = "EOL")
    {
        this->str = std::move(str);
        this->matchesVec.assign(matches.begin(), matches.end());
        this->whiteSpaceSet = std::unordered_set<char>(whiteSpaceList.begin(), whiteSpaceList.end());
        this->endFlags = std::move(endFlags);
    }

    const bool check_whitespace(char ch) const
    {
        return whiteSpaceSet.count(ch) > 0;
    }

    void insert_whitespace(char ch)
    {
        this->whiteSpaceSet.insert(ch);
    }

    LexerResult tokenize()
    {
        std::vector<MatchResult> match;
        match.reserve(str.size()); // Ԥ����ռ�
        size_t pos = 0;
        if (str.empty()) throw "Empty String!";
        size_t len = str.length();

        std::string_view strview(str);

        while (pos < len)
        {
            if (check_whitespace(strview[pos]))
            {
                ++pos;
                continue;
            }

            bool matched = false;
            for (const auto& jt : matchesVec)
            {
                std::match_results<std::string_view::const_iterator> result;
                auto subview = strview.substr(pos);
                if (std::regex_search(subview.begin(), subview.end(), result, jt.match) &&
                    result.position() == 0)
                {
                    std::string resultString(result[0].first, result[0].second);
                    match.emplace_back(resultString, jt.type);
                    pos += result[0].length();
                    matched = true;
                    break;
                }
            }

            if (!matched)
            {
                std::cout << "Unregioned char " << strview[pos] << std::endl;
                ++pos;
            }
        }
        match.emplace_back("", "EOL");
        return LexerResult(match);
    }

private:
    std::string str;
    std::vector<LexerRule> matchesVec;
    std::unordered_set<char> whiteSpaceSet;
    std::string endFlags;
};
