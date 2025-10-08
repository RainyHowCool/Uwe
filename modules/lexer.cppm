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

    /* NOTE: 在我还能看懂时解释一下这个函数的功能
    * 首先，这个函数会创建一个字符串用于存放解析完成的内容
    * ws 变量代表空格, tab 变量代表单 tab, tab2 变量代表双空格, 这三个变量均被 withTab 变量控制( 是否格式化输出 ), result 自带 '{' 字符
    * 对于这个格式化参数，它会将字符串转换成下面的格式
    * ---- 分界线 ----
    * {}( 单 Tab )[
    *   {}( 双 Tab ) "result":{}( 空格位置 )"{}( 结果，也就是匹配到的字符串 )",
    *   {}( 双 Tab ) "type":{}( 空格位置 )"{}( 类型，例如 INTEGER, IDENTIFIER 等 )
    * {}( 单 Tab )]
    * ---- 分界线 ----
    * 在结尾的定义中, result 由 withTab 控制, 如果启用格式化输出会在输出前加入换行符
    * 但无论什么情况，result都会清除最后多余的 ',' 字符
    * 假设输入了字符串 "name:John" 并正确配置了 IDENTIFIER 和 COLON 的正则表达式规则，这是格式化输出例子:
    * ---- 分界线 ----
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
    * ---- 分界线 ----
    * 这是没有被格式化的例子(只有下面一行):
    * {["result":"name","type":"IDENTIFIER"],["result":":","type":"COLON"],["result":"John","type":"IDENTIFIER"]}
    * 希望这一串注释能让你理解这串代码
    */
    std::string get_string(bool withTab = false)
    {
        // INTERESTING IDEA
        std::string tab = withTab ? "\n\t" : "";
        std::string tab2 = withTab ? tab + "\t" : "";
        std::string ws /* White Space */ = withTab ? " " : "";
        std::string result = "{";
        // 遍历结果
        for (const auto& it : this->result)
            result += std::format("{}[{}\"result\":{}\"{}\",{}\"type\":{}\"{}\"{}],",
                tab, tab2, ws, it.result, tab2, ws, it.type, tab);
        // 清除前面的 ',' 字符
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
        match.reserve(str.size()); // 预分配空间
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
