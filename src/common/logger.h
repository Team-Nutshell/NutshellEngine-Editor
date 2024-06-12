#pragma once
#include <vector>
#include <tuple>
#include <ctime>
#include <string>

enum class LogLevel {
	Info,
	Warning,
	Error
};

typedef std::tuple<std::time_t, LogLevel, std::string> Log;

class LogBar;

class Logger {
public:
	void addLog(LogLevel logLevel, const std::string& logMessage);

	const std::vector<Log>& getLogs();

private:
	std::vector<Log> m_logs;

public:
	LogBar* logBar{ nullptr };
};