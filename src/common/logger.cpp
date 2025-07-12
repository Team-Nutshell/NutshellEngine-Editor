#include "logger.h"

Logger::Logger() {
	connect(this, &Logger::clearLogsSignal, this, &Logger::clearLogs);
}

void Logger::addLog(LogLevel logLevel, const std::string& logMessage) {
	 m_logs.push_back({ std::time(nullptr), logLevel, logMessage });
	 emit addLogSignal();
}

const std::vector<Log>& Logger::getLogs() {
	return m_logs;
}

void Logger::clearLogs() {
	m_logs.clear();
}
