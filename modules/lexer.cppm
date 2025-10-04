module;

#include <string>
#include <format>
#include <iostream>
#include <regex>
#include <list>

import logger;

export module lexer;

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
    Lexer(std::string str, std::list<LexerRule> matches, std::vector<char> whiteSpaceList = { '\t', '\r', '\n', ' ' }, std::string endFlags = "EOL")
    {
        this->str = str;
        this->matches = matches;
        this->whiteSpaceList = whiteSpaceList;
        this->endFlags = endFlags;
    }

    // ����Ƿ��ǿո�
    const bool check_whitespace(char ch) const
    {
        return std::count(whiteSpaceList.begin(), whiteSpaceList.end(), ch);
    }

    //���ֵ���в�ֵ
    void insert_whitespace(char ch) 
    {
        this->whiteSpaceList.push_back(ch);
    }

    LexerResult tokenize()
    {
        std::vector<MatchResult> match;
        size_t pos = 0;
        // 1. ����ַ����Ƿ�Ϊ��
        if (str.empty()) throw "Empty String!";
        size_t len = str.length();

        // ��ʼ���ַ�����ƥ��
        while (pos < len)
        {
            bool matched = false;
            // �����ǰ�ַ��ǿ��ַ�������
            if (check_whitespace(str[pos]))
            {
                pos++;
                continue;
            }

            // �ڵ�ǰλ�ó�������ģʽ
            for (const auto& jt : matches)
            {
                // ������
                std::smatch result;
                // ���ڻ�ȡ��ƥ�������
                // TODO: �������� regex_search �л᲻�ᱨ��
                std::string strmatch = str.substr(pos);
                // FIX: ��� result.position ����Է�ֹƥ�䵽���������
                // WHY: Ϊʲô match ������aaaaaaaaaaaaaaaaaa
                if (std::regex_search(strmatch, result, jt.match) &&
                    result.position() == 0) {
                    std::string resultString = result[0].str();
                    match.push_back(MatchResult(resultString, jt.type));
                    pos += result[0].str().length();
                    matched = true;
                    break;
                }
            }

            // ���û��ƥ��ɹ�������в������ַ�����
            if (!matched) 
            {
                std::cout << "Unregioned char " << str[pos] << std::endl;
            }
        }
        match.push_back(MatchResult("", "EOL"));
        LexerResult lexerResult(match);
        return lexerResult;
    }

private:
    // ��Ҫƥ����ַ���
    std::string str;
    // ����ƥ��ģʽ
    std::list<LexerRule> matches;
    // �ո��
    std::vector<char> whiteSpaceList;
    // ������ʶ��
    std::string endFlags;
};
