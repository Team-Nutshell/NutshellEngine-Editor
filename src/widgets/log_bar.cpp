#include "log_bar.h"
#include <QVBoxLayout>

LogBar::LogBar(GlobalInfo& globalInfo) : m_globalInfo(globalInfo) {
	setLayout(new QVBoxLayout());
	layout()->setAlignment(Qt::AlignmentFlag::AlignLeft);
	layout()->setContentsMargins(0, 0, 0, 0);
	lastLogLabel = new QLabel();
	layout()->addWidget(lastLogLabel);
}

void LogBar::updateLastLog(const Log& log) {
	std::string time = std::string(std::asctime(std::localtime(&std::get<0>(log))));
	time.erase(std::remove(time.begin(), time.end(), '\n'), time.end());

	LogLevel logLevel = std::get<1>(log);
	if (logLevel == LogLevel::Info) {
		lastLogLabel->setStyleSheet("color: rgba(225, 225, 225, 255)");
	}
	else if (logLevel == LogLevel::Warning) {
		lastLogLabel->setStyleSheet("color: rgba(225, 115, 0, 255)");
	}
	else if (logLevel == LogLevel::Error) {
		lastLogLabel->setStyleSheet("color: rgba(225, 25, 0, 255)");
	}

	std::string fullLog = "<i>" + time + "</i> - " + std::get<2>(log);
	lastLogLabel->setText(QString::fromStdString(fullLog));
}
