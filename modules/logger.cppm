module;

#include <regex>
#include <fstream>
#include <iostream>
#include <format>
#include <filesystem>

export module logger;

namespace fs = std::filesystem;

constexpr int TRACE = 1;
constexpr int DEBUG = 2;
constexpr int INFO = 3;
constexpr int NOTE = 4;
constexpr int WARN = 5;
constexpr int ERROR = 6;
constexpr int FATAL = 7;

export class Logger
{
public:
	Logger()
	{}

	void trace(std::string text) 
	{
		output(TRACE, text);
	}

	void debug(std::string text)
	{
		output(DEBUG, text);
	}

	void info(std::string text)
	{
		output(INFO, text);
	}

	void note(std::string text)
	{
		output(NOTE, text);
	}

	void warn(std::string text)
	{
		output(WARN, text);
	}

	void error(std::string text)
	{
		output(ERROR, text);
	}

	void fatal(std::string text)
	{
		output(FATAL, text);
	}

	int minOutputLevel = 3;
	size_t maxLogFileSize = 1024576;
	std::string outputFormat = "[TYPE]INFO -> TIME";
	std::string fileName = "info.log";
	std::string timeColor = "\033[93m";
	bool colorEnabled = true;
private:
	const std::string fatal_text = "fatal";
	std::string level2name(int level)
	{
		switch (level)
		{
		case 1:
			return "Trace";
		case 2:
			return "Debug";
		case 3:
			return "Info";
		case 4:
			return "Note";
		case 5:
			return "Warn";
		case 6:
			return "Error";
		case 7:
			return fatal_text;
		default:
			return fatal_text;
		}
	}

	std::string level2color(int level)
	{
		switch (level)
		{
		case 1:
			return "\033[0m";
		case 2:
			return "\033[36m";
		case 3:
			return "\033[34m";
		case 4:
			return "\033[32m";
		case 5:
			return "\033[33m";
		case 6:
			return "\033[31m";
		case 7:
			return "\033[35m";
		default:
			return fatal_text;
		}
	}
	void output(int level, std::string text)
	{
		std::string raw = outputFormat;
		std::string colorRaw = outputFormat;
		std::string levelName = level2name(level);
		time_t now = time(0);
		tm* ltm = localtime(&now);
		std::string time = std::format("{:04d}/{:02d}/{:02d} {:02d}:{:02d}:{:02d}",
			1900 + ltm->tm_year,   // 年份
			1 + ltm->tm_mon,       // 月份（+1因为tm_mon从0开始）
			ltm->tm_mday,          // 日期
			ltm->tm_hour,          // 小时
			ltm->tm_min,           // 分钟
			ltm->tm_sec);          // 秒

		raw = std::regex_replace(raw, std::regex("TYPE"), levelName);
		colorRaw = std::regex_replace(colorRaw, std::regex("TYPE"), level2color(level) + levelName + std::string("\033[0m"));
		raw = std::regex_replace(raw, std::regex("INFO"), text);
		colorRaw = std::regex_replace(colorRaw, std::regex("INFO"), text);
		raw = std::regex_replace(raw, std::regex("TIME"), time);
		colorRaw = std::regex_replace(colorRaw, std::regex("TIME"), timeColor + time + std::string("\033[0m"));

		if (fs::exists(fileName)) 
		{
			size_t size = fs::file_size(fileName);
			if (size > maxLogFileSize)
			{
				std::ofstream file(fileName, std::ios::trunc);
				file.close();
			}
		}
		std::ofstream file(fileName, std::ios::app);
		file << raw << std::endl;
		file.close();
		if (level < minOutputLevel)
			return;
		if (colorEnabled)
			std::cout << colorRaw << std::endl;
		else
			std::cout << raw << std::endl;
		if (levelName == fatal_text)
			exit(1);
	}
};

export Logger Log;