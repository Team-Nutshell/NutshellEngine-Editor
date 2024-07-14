#pragma once
#include <vector>
#include <tuple>
#include <ctime>
#include <string>
#include <QObject>

enum class LogLevel {
	Info,
	Warning,
	Error
};

typedef std::tuple<std::time_t, LogLevel, std::string> Log;

class Logger : public QObject {
	Q_OBJECT
public:
	void addLog(LogLevel logLevel, const std::string& logMessage);

	const std::vector<Log>& getLogs();

private:
	std::vector<Log> m_logs;

signals:
	void addLogSignal();
};