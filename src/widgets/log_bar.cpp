#include "log_bar.h"
#include <QVBoxLayout>

LogBar::LogBar(GlobalInfo& globalInfo) : m_globalInfo(globalInfo) {
	setAlignment(Qt::AlignmentFlag::AlignVCenter);
}

void LogBar::updateLastLog(const Log& log) {
	std::string time = std::string(std::asctime(std::localtime(&std::get<0>(log))));
	time.erase(std::remove(time.begin(), time.end(), '\n'), time.end());

	LogLevel logLevel = std::get<1>(log);
	if (logLevel == LogLevel::Info) {
		setStyleSheet("color: rgba(225, 225, 225, 255)");
	}
	else if (logLevel == LogLevel::Warning) {
		setStyleSheet("color: rgba(225, 115, 0, 255)");
	}
	else if (logLevel == LogLevel::Error) {
		setStyleSheet("color: rgba(225, 25, 0, 255)");
	}

	std::string fullLog = "<i>" + time + "</i> - " + std::get<2>(log);
	setText(QString::fromStdString(fullLog));
}
