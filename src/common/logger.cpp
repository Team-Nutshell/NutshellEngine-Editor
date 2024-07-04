#include "logger.h"
#include "../widgets/log_bar.h"
#include "../widgets/logs_widget.h"

void Logger::addLog(LogLevel logLevel, const std::string& logMessage) {
	 m_logs.push_back({ std::time(nullptr), logLevel, logMessage });
	 if (logBar) {
		 logBar->updateLog();
	 }
	 if (logsWidget) {
		 logsWidget->updateLogs();
	 }
}

const std::vector<Log>& Logger::getLogs() {
	return m_logs;
}
