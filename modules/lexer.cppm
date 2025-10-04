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
    Lexer(std::string str, std::list<LexerRule> matches, std::vector<char> whiteSpaceList = { '\t', '\r', '\n', ' ' }, std::string endFlags = "EOL")
    {
        this->str = str;
        this->matches = matches;
        this->whiteSpaceList = whiteSpaceList;
        this->endFlags = endFlags;
    }

    // 检测是否是空格
    const bool check_whitespace(char ch) const
    {
        return std::count(whiteSpaceList.begin(), whiteSpaceList.end(), ch);
    }

    //向空值表中插值
    void insert_whitespace(char ch) 
    {
        this->whiteSpaceList.push_back(ch);
    }

    LexerResult tokenize()
    {
        std::vector<MatchResult> match;
        size_t pos = 0;
        // 1. 检测字符串是否为空
        if (str.empty()) throw "Empty String!";
        size_t len = str.length();

        // 开始逐字符进行匹配
        while (pos < len)
        {
            bool matched = false;
            // 如果当前字符是空字符则跳过
            if (check_whitespace(str[pos]))
            {
                pos++;
                continue;
            }

            // 在当前位置尝试所有模式
            for (const auto& jt : matches)
            {
                // 储存结果
                std::smatch result;
                // 用于获取可匹配的区域
                // TODO: 看看放在 regex_search 中会不会报错
                std::string strmatch = str.substr(pos);
                // FIX: 添加 result.position 检测以防止匹配到后面的内容
                // WHY: 为什么 match 不能用aaaaaaaaaaaaaaaaaa
                if (std::regex_search(strmatch, result, jt.match) &&
                    result.position() == 0) {
                    std::string resultString = result[0].str();
                    match.push_back(MatchResult(resultString, jt.type));
                    pos += result[0].str().length();
                    matched = true;
                    break;
                }
            }

            // 如果没有匹配成功则代表有不规则字符出现
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
    // 将要匹配的字符串
    std::string str;
    // 所有匹配模式
    std::list<LexerRule> matches;
    // 空格表
    std::vector<char> whiteSpaceList;
    // 结束标识符
    std::string endFlags;
};
